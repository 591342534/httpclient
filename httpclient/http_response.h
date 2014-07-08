/**
 * @file    http_response.h
 * @brief   Http����
 * @author  xiangwangfeng <xiangwangfeng@gmail.com>
 * @data	2011-4-24
 * @website www.xiangwangfeng.com
 */

#pragma once
#include <string>
#include <xstring>
#include "global_defs.h"

NAMESPACE_BEGIN(Http)

//Http����
class HTTP_CLASS HttpResponse
{
public:
	HttpResponse():_http_code(-1) {}
public:
	int						getHttpCode()	const		{	return	_http_code;}//������HTTP״̬��
	const	std::string&	getHeader()	const			{	return	_header;}	//HTTP��Ϣͷ
	const	std::string&	getBody()	const			{	return	_body;}		//HTTP��Ϣ�壬������������ļ�����ʽ�����صģ�����ŵ��ļ�·��(UTF����)
	
private:
	void	setHttpCode(int	http_code)				{	_http_code	=	http_code;}
	void	setBody(const std::string&	body)		{	_body	=	body;}
	void	setHeader(const	std::string&	header)	{	_header	=	header;}

private:
	std::string		_header;					//Httpͷ
	std::string		_body;						//Http��
	int				_http_code;					//Http������
	friend	class	HttpClient;
};

NAMESPACE_END(Http)