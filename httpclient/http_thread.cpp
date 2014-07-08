/**
 * @file    http_thread.cpp
 * @brief   Http�߳�
 * @author  xiangwangfeng <xiangwangfeng@gmail.com>
 * @data	2011-7-8
 * @website www.xiangwangfeng.com
 */

#include "standard_header.h"
#include "http_thread.h"
#include "http_delegate.h"
#include "http_request.h"
#include "http_response.h"
#include "http_client.h"
#include "http_pool.h"

NAMESPACE_BEGIN(Http)


HttpThread::HttpThread(const std::string& name,HttpPool* http_pool)
:Util::Thread(name),
_http_pool(http_pool)
{
	_http_client	=	new HttpClient();	
}

HttpThread::~HttpThread()
{
	delete _http_client;
}

void	HttpThread::run()
{
	HANDLE notify_handle[2];
	notify_handle[0]	=	_http_pool->_exit_event;
	notify_handle[1]	=	_http_pool->_work_semaphore;

	while (true)
	{
		DWORD object = WaitForMultipleObjects(2,notify_handle,FALSE,INFINITE);
		if (object == WAIT_OBJECT_0)
		{
			break;	//�˳�
		}
		else if (object == WAIT_OBJECT_0 + 1)
		{
			executeHttp();//ִ������
		}
		else
		{
			assert(false);
			break;	//�����ϲ��ᵽ��һ��
		}
	}
}

void	HttpThread::killSelf()
{
	_http_client->killSelf();
}

void	HttpThread::executeHttp()
{
	HttpWorkItem*	item = _http_pool->getWork();
	if (item)
	{
		IAsyncHttpDelegate* delegate = item->_async_delegate;

		//����HTTP����
		_http_client->reset();
		_http_client->setProgressDelegate(delegate);
		HttpResponse response;
		bool execute = _http_client->execute(item->_http_request,&response);
		//ִ�лص�
		if (delegate)
		{
			if (execute)
			{
				delegate->onSuccess(item->_http_request,&response);
			}
			else
			{
				delegate->onError(_http_client->getErrorCode(),item->_http_request,&response);
			}
		}

		//������Դ
		delete item;
	}
}


NAMESPACE_END(Http)
