/**
 * @file    file_reader.h
 * @brief   ���ļ�Wrapper��
 * @author  xiangwangfeng <xiangwangfeng@gmail.com>
 * @data	2011-4-27
 * @website www.xiangwangfeng.com
 */

#pragma once

#include <xstring>
#include "global_defs.h"

NAMESPACE_BEGIN(Util)

//���ļ�Wrapper��
class	FileReader
{
public:
	FileReader(const std::wstring&	filepath);
	~FileReader();
public:
	bool	open();
	int		read(char* buffer,size_t length);
	void	close();
private:
	std::wstring	_filepath;		//�ļ�·��
	bool			_ready;			//�Ƿ�׼�����
	FILE*			_file;			//�ļ�ָ��
};

NAMESPACE_END(Util)