/**
 * @file    http_client.h
 * @brief   Http������
 * @author  xiangwangfeng <xiangwangfeng@gmail.com>
 * @data	2011-4-24
 * @website www.xiangwangfeng.com
 */

#pragma once
#include <string>
#include "global_defs.h"
#include "http_global.h"
#include "proxy_config.h"

namespace Util
{
	class Lock;
}

NAMESPACE_BEGIN(Http)

class	ProxySocket;
class	HttpRequest;
class	HttpResponse;
class	IHttpPostFile;
class	IProgressDelegate;

//Http������
class	HTTP_CLASS	HttpClient
{
public:
	HttpClient(bool keep_connection = false);
	~HttpClient();
public:
	//ִ��Http��������
	bool			execute(HttpRequest* request,HttpResponse* respone);							//ִ��Http����
	static	void	setProxy(const ProxyConfig*	proxy_config);										//���ô���
	HTTPERROR		getErrorCode()	const {	return	_http_error;}									//���ش�����
	void			setProgressDelegate(IProgressDelegate* delegate)	{ _delegate = delegate;}	//���ý���ί��
private:
	void		killSelf();																		//���õ�ǰHttpClient
	void		reset();																		//���赱ǰHttpClient
	void		setErrorCode(HTTPERROR http_error);
	bool		httpGet();
	bool		httpPost();
	bool		getResponse();
	bool		downloadHeader(std::string&	body_header);
	bool		downloadBody(const std::string& body_header);
	bool		downloadFixedSizeBody(const std::string& body_header,int content_length);
	bool		downloadChunkedBody(const std::string& body_header);
	bool		continueToReceiveBody(std::string&	body);
	bool		sendHeader();
	bool		sendBody();
	bool		doMultipartPost();
	bool		uploadFile(IHttpPostFile*	post_file);
	void		onDataReadProgress(int read_length,int total_length);
	DECLARE_NON_COPYABLE(HttpClient)
private:
	ProxySocket*			_proxy_socket;			//֧�ִ����Socket��
	HTTPERROR				_http_error;			//Http���������
	bool					_keep_connection;		//�Ƿ񱣳�����
	HttpRequest*			_request;				//Http���󷽷�
	HttpResponse*			_response;				//Http��������
	IProgressDelegate*		_delegate;				//Http�����ί��
	bool					_is_valid;				//�Ƿ����(���������killSelf�ӿ����Client������)
	Util::Lock*				_is_valid_lock;			//�Ƿ���õ�ͬ����

	friend	class HttpThread;
};

NAMESPACE_END(Http)