/**
 * @file    streaming_socket.h
 * @brief   ��ԭ��WinSocket�ķ�װ��
 * @author  xiangwangfeng <xiangwangfeng@gmail.com>
 * @data	2011-4-23
 * @website www.xiangwangfeng.com
 */

#pragma once
#include <string>
#include "global_defs.h"

NAMESPACE_BEGIN(Http)

//��ԭ��SOCKET�ķ�װ
class HTTP_CLASS StreamingSocket
{
public:
	StreamingSocket();
	~StreamingSocket();
public:
	bool	connect(const std::string& remote_host_name,
					int remote_port_number,
					int timeout = 3000);

	bool	isConnected() const	{return _connected;}

	void	close();

	int		read(char* dest_buffer,int max_read_length);

	int		write(const char* source_buffer,int max_write_length);

	bool	writeAll(const char* source_buffer,int max_write_length);
private:
	bool	resetSocketOptions();
private:
	std::string		_host_name;				//Ŀ���ַ
	int	volatile	_port_number,_handle;	//�˿ںţ�SOCKET
	bool			_connected;				//�Ƿ�������
	
};


NAMESPACE_END(Http)