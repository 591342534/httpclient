/**
 * @file    http_request.cpp
 * @brief   Http����
 * @author  xiangwangfeng <xiangwangfeng@gmail.com>
 * @data	2011-4-24
 * @website www.xiangwangfeng.com
 */
#include "standard_header.h"
#include "http_request.h"
#include <ctime>
#include "http_global.h"
#include "util.h"
#include "url_parser.h"
#include "http_post_file.h"


NAMESPACE_BEGIN(Http)


HttpFile::~HttpFile()
{
	delete _post_file;
}


HttpRequest::HttpRequest()
:_is_multipart(false),
_port_number(80),
_only_download_header(false),
_save_as_file(false)
{
	initDefaultValue();
}

HttpRequest::HttpRequest(const std::string& method,const std::string& url)
:_is_multipart(false),
_port_number(80),
_only_download_header(false),
_save_as_file(false)
{
	initDefaultValue();
	setRequest(method,url);
}

HttpRequest::~HttpRequest()
{

}

void	HttpRequest::setRequest(const std::string& method,const std::string& url)
{
	if (_strcmpi(method.c_str(),kget) == 0)
	{
		_http_method	=	kget;
	}
	else if (_strcmpi(method.c_str(),kpost) == 0)
	{
		_http_method	=	kpost;
	}
	else
	{
		assert(false);
		return;
	}
	URLParser	url_parser(url);
	_http_host	=	url_parser.getDomain();
	_object		=	url_parser.getObject();
	_port_number=	url_parser.getPort();
}

void	HttpRequest::addHeaderField(const std::string &name, const std::string &value)
{
	for (size_t	i = 0; i < _header_fields.size(); i++)
	{
		std::string& header_name	=	_header_fields[i]._field_name;
		if (_strcmpi(header_name.c_str(),name.c_str()) == 0)
		{
			//�����ֶ�ֱ������ֵ�����ֵ
			_header_fields[i]._field_value	=	value;
			return;
		}
	}
	_header_fields.push_back(HttpField(name,value));
}



void	HttpRequest::addFile(const std::string& name,IHttpPostFile*	post_file)
{
	PTR_VOID(post_file);
	for (size_t i = 0; i < _post_files.size(); i ++)
	{
		const std::string&	add_name	=	_post_files[i]->_name;
		if (_strcmpi(name.c_str(),add_name.c_str()) == 0)
		{
			//����������
			assert(false);
			return;
		}
	}
	HttpFile* http_file = new HttpFile(name,post_file);
	_post_files.push_back(http_file);
	_is_multipart = true;
}


void	HttpRequest::initDefaultValue()
{
	_http_method	=	kget;
	
	//���һЩĬ��Httpͷ�ֶ�
	_header_fields.push_back(HttpField(kaccept,"*/*"));
	_header_fields.push_back(HttpField(kconnection,"Keep-Alive"));
}


int		HttpRequest::generateHeader(std::string& header)
{
	header.clear();


	//���󷽷���ʵ��
	header.append(_http_method+" ");
	header.append(_object);
	header.append(" HTTP/1.1\r\n");

	//����
	header.append("Host:");
	header.append(_http_host);
	header.append("\r\n");


	//����Body�ĳ��ȣ���ֵ��ContentLength
	if (_stricmp(_http_method.c_str(),kpost) == 0)
	{
		calcBody();
	}

	//���Header Field
	size_t	field_size	=	_header_fields.size();
	for (size_t i = 0; i < field_size; i ++)
	{
		std::string&	name	=	_header_fields[i]._field_name;
		std::string&	value	=	_header_fields[i]._field_value;
		header.append(name	+	": " +	value	+	"\r\n");
	}
	
	//������һ��\r\n
	header.append("\r\n");

	///���ؽ��
	return (int)header.size(); 
}

void	HttpRequest::calcBody()
{
	if (_is_multipart)
	{
		genBoundary();

		int length = 0; //����Content-Length
		_body.clear();


		//���Fields
		for (size_t i = 0; i < _body_fields.size(); i++)
		{
			const std::string&	name	=	_body_fields[i]._field_name;
			const std::string&	value	=	_body_fields[i]._field_value;
			std::string	field			=	"--" + _boundary + "\r\n" +
											"content-disposition: form-data; name=\"" + name + "\"\r\n\r\n" +
											 value + "\r\n";
			_body						+=	field;
		}
		length	=	(int)_body.size();

		//���Files
		for (size_t i = 0; i < _post_files.size(); i++)
		{
			std::string name			=	_post_files[i]->_name;
			IHttpPostFile* post_file	=	_post_files[i]->_post_file;
			std::string post_part		=	"--" + _boundary + "\r\n" + 
											"content-disposition: form-data; name=\"" + name + 
											"\"; filename=\"" + post_file->getFilename() + "\"\r\n" +
											"content-type: " + post_file->getContentType() + "\r\n" +
											"\r\n";
			length			+= (int)post_part.size();
			length			+= post_file->getFileSize();
			length			+= 2;						//���ļ�����Ҫ���\r\n
		}

		std::string post_end = "--" + _boundary + "--\r\n";
		length += (int)post_end.size();

		//���Content-Type��Content-Length�ֶ�
		std::string	content_type = "multipart/form-data; boundary=" + _boundary;
		addHeaderField(kcontent_type,content_type);
		std::string	content_length	=	Util::num_to_string(length);
		addHeaderField(kcontent_length,content_length);
	}
	else
	{
		//���û��ֱ������Http�壬����Field����ʽ����HTTP��
		if (_body.empty())
		{
			for (size_t i = 0 ; i < _body_fields.size(); i++)
			{	
				if (!_body.empty())
				{
					_body	+=	"&";
				}
				const std::string&	name	=	_body_fields[i]._field_name;
				const std::string&	value	=	_body_fields[i]._field_value;
				_body						+=	(name + "=" + Util::torfc1738(value));
			}
			addExtraHeaderField(kcontent_type,"application/x-www-form-urlencoded");
		}
		else
		{
			addExtraHeaderField(kcontent_type,"text/plain");
		}
		std::string	content_length	=	Util::num_to_string((int)_body.length());
		addHeaderField(kcontent_length,content_length);
	}

}

void	HttpRequest::genBoundary()
{
	_boundary = "----";

	std::string random_number;
	unsigned char c;
	srand((unsigned int)time(0));
	for (int i = 1; i < 15; i++)
	{
		c = int(rand()*127);
		if (isalpha(c) || isdigit(c))
		{
			random_number += std::string(1,(char)c);
		}
		else
		{
			i--;
		}
	}
	_boundary += "__" + random_number;
}

void	HttpRequest::addExtraHeaderField(const std::string& name,const std::string& value)
{

	for (size_t	i = 0; i < _header_fields.size(); i++)
	{
		std::string& header_name	=	_header_fields[i]._field_name;
		if (_strcmpi(header_name.c_str(),name.c_str()) == 0)
		{
			//����Ѿ���ͬ����header ֱ�ӷ��� �����header field
			return;
		}
	}
	_header_fields.push_back(HttpField(name,value));
}


NAMESPACE_END(Http)