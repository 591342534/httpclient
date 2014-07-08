/**
 * @file    http_pool.h
 * @brief   Http�����
 * @author  xiangwangfeng <xiangwangfeng@gmail.com>
 * @data	2011-7-6
 * @website www.xiangwangfeng.com
 */
#pragma once
#pragma warning(disable:4251)

#include <vector>
#include <list>
#include "global_defs.h"

NAMESPACE_BEGIN(Util)
class Lock;
NAMESPACE_END(Util)

NAMESPACE_BEGIN(Http)

class	HttpPool;
class	HttpClient;
class	HttpRequest;
class	IAsyncHttpDelegate;
class	HttpThread;

struct HttpWorkItem 
{
	HttpRequest*		_http_request;
	IAsyncHttpDelegate*	_async_delegate;
	HttpWorkItem(HttpRequest* http_request,IAsyncHttpDelegate* delegate);
	~HttpWorkItem();
};


class HTTP_CLASS HttpPool	
{
public:
	~HttpPool();
	static HttpPool*	getInstance();
	static void			freeInstance();
	
	//�첽����HTTP���䣬�ⲿ�����Լ�����http_request��delegate��������
	void				postRequest(HttpRequest* http_request,IAsyncHttpDelegate* delegate);
private:
	HttpPool();
	void		init();
	void		releaseAll();
	HttpWorkItem*	getWork();
	std::string		genThreadName(int index);
private:
	std::vector<HttpThread*>	_http_threads;		//�߳��б�
	std::list<HttpWorkItem*>	_http_work_list;	//��������
	static	HttpPool*			_pool;				//��̬�ض���
	void*						_exit_event;		//�˳��¼�
	void*						_work_semaphore;	//�����ź���
	Util::Lock*					_work_lock;			//�̳߳ض��е���
	friend	class	HttpThread;
};

NAMESPACE_END(Http)
