/**
 * @file    url_parser.h
 * @brief   �򵥵�URL������(ȱ�ٸ����жϣ�������)
 * @author  xiangwangfeng <xiangwangfeng@gmail.com>
 * @data	2011-4-24
 * @website www.xiangwangfeng.com
 */

#pragma once
#include <string>
#include "global_defs.h"

NAMESPACE_BEGIN(Http)

//URL������
class URLParser
{
public:
	URLParser(const std::string&	url);
public:
	const	std::string&	getDomain()	const	{	return	_domain;}
	const	std::string&	getObject()	const	{	return	_object;}
	int		getPort()	const {	return	_port;}
private:
	void	parseURL(const std::string&	url);
	void	splitURL(const std::string& format_url);
private:
	int			_port;		//�˿�
	std::string	_domain;	//����
	std::string	_object;	//��������
};

NAMESPACE_END(Http)