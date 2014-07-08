/**
 * @file    http_client.cpp
 * @brief   Http������
 * @author  xiangwangfeng <xiangwangfeng@gmail.com>
 * @data	2011-4-24
 * @website www.xiangwangfeng.com
 */
#include "standard_header.h"
#include "http_client.h"
#include "util.h"
#include "lock.h"
#include "proxy_socket.h"
#include "http_request.h"
#include "http_response.h"
#include "http_delegate.h"
#include "file_writer.h"
#include "http_header_parser.h"
#include "http_response_receiver.h"
#include "http_post_file.h"

NAMESPACE_BEGIN(Http)

HttpClient::HttpClient(bool keep_connection)
:_http_error(HTTPERROR_PARAMETER),
_keep_connection(keep_connection),
_request(0),
_response(0),
_delegate(0),
_is_valid(true)
{
	_proxy_socket	=	new ProxySocket();
	_is_valid_lock	=	new Util::Lock();
}

HttpClient::~HttpClient()
{
	delete	_proxy_socket;
	delete	_is_valid_lock;
}

bool	HttpClient::execute(HttpRequest* request,HttpResponse* respone)
{
	//�������
	PTR_BOOL(request);
	PTR_BOOL(respone);
	_request	=	request;
	_response	=	respone;
	setErrorCode(HTTPERROR_SUCCESS);

	//���ӷ�����
	const std::string&	host	=	_request->getHost();
	int	port_number				=	_request->getPortNumber();
	_proxy_socket->setHost(host,port_number);

	//��������
	{
		const Util::ScopedLock scoped_lock(*_is_valid_lock);
		if (_is_valid)	//�ж�HttpClient�Ƿ���Ч
		{
			if (!(_keep_connection && _proxy_socket->isConnected()))
			{
				if (!_proxy_socket->connect())
				{
					setErrorCode(HTTPERROR_CONNECT);
					return false;
				}
			}
		}
		else
		{
			setErrorCode(HTTPERROR_INVALID);
			return false;
		}
	}

	//�������ݴ���
	bool	execute				=	false;
	const	std::string& method	=	_request->getMethod();

	//Get����
	if (_strcmpi(method.c_str(),kget) == 0)
	{
		execute	=	httpGet();
	}
	//Post����
	else if (_strcmpi(method.c_str(),kpost) == 0)
	{
		execute	=	httpPost();
	}
	//�������׳�����
	else
	{
		assert(false);
	}
	
	//������ǳ����� �͹ر�����
	if (!_keep_connection)
	{
		_proxy_socket->close();
	}

	return execute;
}

void	HttpClient::setProxy(const Http::ProxyConfig *proxy_config)
{
	PTR_VOID(proxy_config);
	ProxySocket::setProxy(*proxy_config);
}

void	HttpClient::setErrorCode(HTTPERROR http_error)
{
	_http_error	=	http_error;
	//�������ʧ�ܺ󣬹رյ�ǰSocket 
	//(Ϊ��֧��keep-aliveģʽ���ڷ�����������ܹ�������������)
	if (_http_error != HTTPERROR_SUCCESS)	
	{
		_proxy_socket->close();
	}
}


void	HttpClient::killSelf()
{
	const Util::ScopedLock scoped_lock(*_is_valid_lock);
	_is_valid	=	false;
	_proxy_socket->close();
}

void	HttpClient::reset()
{
	const Util::ScopedLock scoped_lock(*_is_valid_lock);
	_proxy_socket->close();
}


bool	HttpClient::httpGet()
{
	//����HTTPͷ����
	if (sendHeader())
	{
		//���ܷ���
		return	getResponse();
	}
	else
	{
		setErrorCode(HTTPERROR_TRANSPORT);
		return false;
	}
}

bool	HttpClient::httpPost()
{
	//����HTTPͷ����
	bool complete = false;
	if (sendHeader())
	{
		if (_request->isMultipart())
		{
			complete	=	doMultipartPost();
		}
		else
		{
			if (sendBody())
			{
				complete	= 	getResponse();
			}
		}
	}
	return complete;
}

bool	HttpClient::sendHeader()
{
	std::string header;
	int	header_length	=	_request->generateHeader(header);
	bool send			=	_proxy_socket->writeAll(header.c_str(),header_length);
	if (!send)
	{
		setErrorCode(HTTPERROR_TRANSPORT);
	}
	return send;
}

bool	HttpClient::sendBody()
{
	const std::string&	body	=	_request->getBody();
	size_t	length				=	body.length();
	if (length == 0)
	{
		return true;	//���body��û������ ֱ�ӷ���true
						//(�ϴ��ļ���ʱ��body�����Ƕ�̬���ɣ�body���ݿ���Ϊ��) 
	}
	bool	send				=	_proxy_socket->writeAll(body.c_str(),body.length());
	if (!send)
	{
		setErrorCode(HTTPERROR_TRANSPORT);;
	}
	return send;
}

bool	HttpClient::doMultipartPost()
{
	//�����Fields�� �Ѿ�д����body��  ֱ�ӷ���
	if (!sendBody())
	{
		setErrorCode(HTTPERROR_TRANSPORT);
		return false;
	}

	//�����ļ�
	const std::vector<HttpFile*>& post_files	=	_request->getFiles();
	const std::string&	boundary				=	_request->getBoundary();
	for (size_t i = 0; i < post_files.size(); i++)
	{
		const std::string name		=	post_files[i]->_name;
		IHttpPostFile* post_file	=	post_files[i]->_post_file;
		std::string file_header		=  "--" + boundary + "\r\n"
										"content-disposition: form-data; name=\"" + name + "\"; filename=\"" + 
										post_file->getFilename() + "\"\r\n" +
										"content-type: " + post_file->getContentType() + "\r\n" +
										"\r\n";

		bool send_file_header		 = _proxy_socket->writeAll(file_header.c_str(),file_header.size());
		if (!send_file_header)
		{
			setErrorCode(HTTPERROR_TRANSPORT);
			return false;
		}

		bool post_file_success = uploadFile(post_file);
		if (!post_file_success)
		{
			setErrorCode(HTTPERROR_TRANSPORT);
			return false;
		}


		std::string file_tailer = "\r\n";
		bool send_file_tailer	 = _proxy_socket->writeAll(file_tailer.c_str(),file_tailer.size());
		if (!send_file_tailer)
		{
			setErrorCode(HTTPERROR_TRANSPORT);
			return false;
		}
	}

	//����boundary�������
	std::string post_tailer		= "--" + boundary + "--\r\n";
	bool send_post_tailer		= _proxy_socket->writeAll(post_tailer.c_str(),post_tailer.size());
	
	return send_post_tailer ? getResponse() : setErrorCode(HTTPERROR_TRANSPORT) , false;
	
}

bool	HttpClient::uploadFile(IHttpPostFile* post_file)
{
	FilePoster file_poster(_proxy_socket,_http_error,_delegate);
	return post_file ? post_file->postFile(file_poster) : false;
}


bool	HttpClient::getResponse()
{
	std::string	body_header;		//��ȡ��Body��Header�Ļ����
	if (downloadHeader(body_header))
	{
		//�������ָ��ֻ��Ҫ��ȡHttpͷֱ�ӷ��� (��Ҫ��Ϊ�ֶ����ؼ���������)
		if (_request->onlyDownloadHeader())
		{
			return true;
		}
		else
		{
			return	downloadBody(body_header);
		}
	}
	else
	{
		return	false;
	}
}

bool	HttpClient::downloadHeader(std::string& body_header)
{
	body_header.clear();
	char buff[kmax_buffer_size] = {0};
	std::string	header;
	bool complete = false;
	while(!complete)
	{
		int ret = _proxy_socket->read(buff,kmax_buffer_size);
		if (ret <= 0)
		{
			setErrorCode(HTTPERROR_TRANSPORT);
			break;
		}
		header.append(buff,ret);					//��ΪHeader�����̣ܶ�����һ�ο�������
		size_t end_index = header.find("\r\n\r\n");	//����Ҳ����Ҫ����ƫ������������ٶ�
		if (end_index != std::string::npos)
		{
			complete			= true;
			size_t length		= header.length() ;
			body_header			= header.substr(end_index + 4,length - end_index - 4);
			_response->setHeader(header.substr(0,end_index + 4));
		}
	}
	return complete;
}

bool	HttpClient::downloadBody(const std::string& body_header)
{
	const std::string& header	=	_response->getHeader();
	HttpHeaderParser	header_parser(header);
	bool	is_chunked			=	header_parser.isChunked();
	int		http_code			=	header_parser.getHttpCode();
	int		content_lenght		=	header_parser.getContentLength();
	bool	complete			=	false;
	_response->setHttpCode(http_code);
	if (is_chunked)	//Chunk���͵�Http��
	{
		complete = downloadChunkedBody(body_header);
	}
	else	//��Content-Length��Http��
	{
		complete = downloadFixedSizeBody(body_header,content_lenght);	
	}

	return complete;	//�������
}


bool	HttpClient::downloadFixedSizeBody(const std::string& body_header,int content_length)
{
	int	received_length	=	(int)body_header.length();
	if (received_length > content_length ||
		content_length	<	0)
	{
		setErrorCode(HTTPERROR_TRANSPORT);
		assert(false);
		return false;
	}
	
	//����Http����������
	HttpResponseReceiver*	response_receiver	=	0;
	if (_request->saveAsFile())
	{
		const std::wstring& filepath	=	_request->getFilePath();
		_response->setBody(Util::toUTF8(filepath));		//��������ص�����IO����response��Body���汣������ļ�·��
		response_receiver				=	new HttpResponseReceiver(filepath);
	}
	else
	{
		response_receiver				=	new HttpResponseReceiver();
	}

	//����Http��
	bool	complete=	false;
	bool	write	=	response_receiver->write(body_header.c_str(),body_header.length());
	if (write)
	{
		int		unreceived_length			=	content_length	-	received_length;
		onDataReadProgress(received_length,content_length);
		char	buff[kmax_file_buffer_size]	=	{0};
		while (unreceived_length >	0)
		{
			int	ret	=	_proxy_socket->read(buff,kmax_file_buffer_size);
			if (ret <= 0)
			{
				setErrorCode(HTTPERROR_TRANSPORT);
				break;
			}
			else if (!response_receiver->write(buff,ret))
			{
				setErrorCode(HTTPERROR_IO);
				break;
			}
			unreceived_length	-=	ret;
			onDataReadProgress(content_length - unreceived_length,content_length);
		}
		complete	=	(unreceived_length	==	0);
	}
	else
	{
		setErrorCode(HTTPERROR_IO);
	}

	//�����д���ڴ����� ��Ҫ���ظ�HttpResponse
	if (complete && !_request->saveAsFile())
	{
		const std::string&	body	=	response_receiver->getBody();
		_response->setBody(body);
	}

	delete	response_receiver;
	return	complete;
}

bool	HttpClient::downloadChunkedBody(const std::string& body_header)
{
	std::string body		= body_header;		//����HTTPͷʱ�õ��Ĳ���HTTP����Ϣ
	bool complete			= false;			//�Ƿ��Ѿ�����������
	bool find_first_chunk	= false;			//�Ƿ��ҵ���һ��ChunkSize
	int	 chunk_size			= 0;				//Chunk�����ݴ�С
	int  chunk_size_length	= 0;				//ChunkSize�Ĵ�С  ���� 12\r\n ˵��;chunk_sizeΪ18 chunk_size_lengthΪ2

	//����Http����������
	HttpResponseReceiver*	response_receiver	=	0;
	bool	save_as_file						=	_request->saveAsFile();
	if (save_as_file)
	{
		const std::wstring& filepath	=	_request->getFilePath();
		_response->setBody(Util::toUTF8(filepath));
		response_receiver				=	new HttpResponseReceiver(filepath);
	}
	else
	{
		response_receiver				=	new HttpResponseReceiver();
	}

	//���ܲ�����chunk����
	while(true)
	{
		//������ϴ��Ѿ���ѯ����һ��chunk�Ĵ�С
		if (find_first_chunk)
		{
			if (chunk_size == 0)//��������һ����
			{
				complete = true;
				break;
			}
			else	//�������chunk���ݲ������и�
			{
				size_t length		= body.length();
				size_t first_chunk	= chunk_size_length + 2 + chunk_size + 2;
				if (length >= first_chunk)	//����Ѿ����ܵ�һ����chunkdata��������и�������½���
				{
					find_first_chunk		= false;
					std::string chunk_data	= body.substr(chunk_size_length + 2, chunk_size);
					body.erase(0,first_chunk);
					if (!response_receiver->write(chunk_data.c_str(),chunk_data.length()))
					{
						setErrorCode(HTTPERROR_IO);
						break;
					}
				}
				else
				{
					if (!continueToReceiveBody(body))
					{
						setErrorCode(HTTPERROR_TRANSPORT);
						break;
					}
				}
			}
		}
		else//����chunk_size
		{
			size_t index = body.find("\r\n");
			if (index != std::string::npos)		//�ҵ��������
			{
				find_first_chunk			= true;
				chunk_size_length			= (int)index;
				std::string raw_chunk_size	= body.substr(0,chunk_size_length);
				chunk_size					= (int)strtoul(raw_chunk_size.c_str(),0,16);

			}
			else	//û���ҵ�������������Ϣ
			{
				if (!continueToReceiveBody(body))
				{
					setErrorCode(HTTPERROR_TRANSPORT);
					break;
				}
			}
		}
	}

	if (!save_as_file && complete)
	{
		const std::string&	body	=	response_receiver->getBody();
		_response->setBody(body);
	}
	delete response_receiver;

	return complete;	
}

bool	HttpClient::continueToReceiveBody(std::string& body)
{
	char buff[kmax_file_buffer_size] = {0};
	int ret	=	_proxy_socket->read(buff,kmax_file_buffer_size);
	if (ret <= 0)
	{
		setErrorCode(HTTPERROR_TRANSPORT);
		return false;
	}
	else
	{
		body.append(buff,ret);
		return true;
	}
}

void	HttpClient::onDataReadProgress(int read_length,int total_length)
{
	if (_delegate) 
	{
		_delegate->dataReadProgress(read_length,total_length);
	}
}

NAMESPACE_END(Http)