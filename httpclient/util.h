/**
 * @file    util.h
 * @brief   ͨ�õĸ�����������
 * @author  xiangwangfeng <xiangwangfeng@gmail.com>
 * @data	2011-4-23
 * @website www.xiangwangfeng.com
 */

#pragma once
#include <string>
#include "global_defs.h"



NAMESPACE_BEGIN(Util)

//ת���ַ���ΪСд
void	makeLower(char*	source,int len);

//ת���ַ���ΪСд
void	makeLower(const std::string& input,std::string& output);

//����ת�ַ���
std::string	num_to_string(int num);

//��ȥ�ַ������߿ո�
void	trimString(const std::string& input,std::string& output);

//UnicodeתUTF8
std::string	toUTF8(const std::wstring&	source);

//UnicodeתMBCS
std::string	toMBCS(const std::wstring&	source);

//�����ļ���С
int		getFileSize(const std::wstring&	filepath);

//rfc1738����
std::string torfc1738(const std::string& source);

NAMESPACE_END(Util)