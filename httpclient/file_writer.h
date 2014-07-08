/**
 * @file    file_writer.h
 * @brief   д�ļ�Wrapper��
 * @author  xiangwangfeng <xiangwangfeng@gmail.com>
 * @data	2011-4-25
 * @website www.xiangwangfeng.com
 */

#pragma once

#include <xstring>
#include "global_defs.h"

NAMESPACE_BEGIN(Util)

//д�ļ�Wrapper��
class FileWriter
{
public:
	FileWriter(const std::wstring&	filepath);
	~FileWriter();
public:
	bool	create();
	bool	write(const char* buffer,size_t length);
	void	flush();
	void	close();
private:
	std::wstring	_filepath;		//�ļ�·��	
	bool			_ready;			//�Ƿ�׼�����
	FILE*			_file;			//�ļ�ָ��
};


NAMESPACE_END(Util)