/**
 * @file    proxy_socket.h
 * @brief   ֧�ִ����Socket��
 * @author  xiangwangfeng <xiangwangfeng@gmail.com>
 * @data	2011-4-23
 * @website www.xiangwangfeng.com
 */

#pragma once
#include <string>
#include "proxy_config.h"
#include "streaming_socket.h"

NAMESPACE_BEGIN(Http)

//֧�ִ����Socket���װ
class HTTP_CLASS ProxySocket
{
public:
	ProxySocket();
	ProxySocket(const std::string& remote_host_name,int port_number);
	~ProxySocket();

public:
	void	setHost(const std::string& remote_host_name,int port_number);
	static	void	setProxy(const ProxyConfig& proxy_config);

	bool	connect();
	bool	isConnected()	const	{	return	_socket.isConnected();}
	int		read(char* dst_buffer,int max_read_length);
	int		write(const char* source_buffer,int max_write_length);
	bool	writeAll(const char* source_buffer,int max_write_length);	//ȷ���������ݶ�������
	void	close();
private:
	bool	handShakeWithHttpProxy();
	bool	handShakeWithSock4Proxy();
	bool	handShakeWithSock5Proxy();
private:
	std::string			_host_name;
	int					_port_number;
	StreamingSocket		_socket;
	static ProxyConfig	_proxy_config;
};


NAMESPACE_END(Http)