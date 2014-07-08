/**
 * @file    http_header_parser.h
 * @brief   Httpͷ������
 * @author  xiangwangfeng <xiangwangfeng@gmail.com>
 * @data	2011-4-24
 * @website www.xiangwangfeng.com
 */

#pragma once
#include <string>
#include "global_defs.h"

NAMESPACE_BEGIN(Http)

//Http�ļ�ͷ������
//��������ಢ����һ��ͨ�õĽ����ֻ࣬��ע�����ֶΣ�
//�Ƿ�Chunked���䣬�ļ����ȣ�Http������
class HttpHeaderParser
{
public:
	HttpHeaderParser(const std::string& header);
public:
	bool	isChunked()	const			{	return	_chunked;}
	int		getContentLength()	const	{	return	_content_length;}
	int		getHttpCode()	const		{	return	_http_code;}
private:
	void	parseHeader();
	void	parseChunkedType();
	void	parseConetentLength();
	void	parseHttpCode();
private:
	std::string		_header;
	bool			_chunked;
	int				_http_code;
	int				_content_length;
};

NAMESPACE_END(Http)