/**
 * @file    thread.h
 * @brief   �̷߳�װ
 * @author  xiangwangfeng <xiangwangfeng@gmail.com>
 * @data	2011-7-6
 * @website www.xiangwangfeng.com
 * @note �����ֻ���ڲ�ʹ�� ���Բ��ṩ���߳�ǿɱ�ķ�����Ҳ���Ƽ�ʹ��
 */
#pragma once
#include <string>
#include "global_defs.h"

NAMESPACE_BEGIN(Util)

class Thread
{
public:
	explicit Thread(const std::string& thread_name);
	virtual ~Thread();
public:
	void	waitThreadExit();
	void	startThread();
	void	enterThread();
	virtual	void	run()	=	0;
	static	unsigned int __stdcall	ThreadProc(void* data);
private:
	void	setThreadName();
	void	launchThread();
protected:
	std::string		_thread_name;
	void *			_thread_handle;
	unsigned int	_thread_id;
	
};

NAMESPACE_END(Util)