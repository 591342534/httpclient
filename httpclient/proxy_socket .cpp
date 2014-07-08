/**
 * @file    proxy_socket.cpp
 * @brief   ֧�ִ����Socket��
 * @author  xiangwangfeng <xiangwangfeng@gmail.com>
 * @data	2011-4-23
 * @website www.xiangwangfeng.com
 */

#include "standard_header.h"
#include "proxy_socket.h"
#include "base64.h"
#include "util.h"
#include "socket_helper.h"

#pragma pack(1)	//1�ֽ��ڴ����ģʽ(�����䶼���Խṹ��ֱ��ת�����ֽ��������Ա���1�ֽڶ���)

NAMESPACE_BEGIN(Http)

ProxyConfig	ProxySocket::_proxy_config;

ProxySocket::ProxySocket()
:_port_number(0)
{

}

ProxySocket::ProxySocket(const std::string &remote_host_name, 
						 int port_number)
						 :_host_name(remote_host_name),_port_number(port_number)
{
	
}

ProxySocket::~ProxySocket()
{
	close();
}


void	ProxySocket::setHost(const std::string &remote_host_name, int port_number)
{
	_host_name		=	remote_host_name;
	_port_number	=	port_number;
}

void	ProxySocket::setProxy(const ProxyConfig &proxy_config)
{
	_proxy_config.copy(proxy_config);
}


bool	ProxySocket::connect()
{
	ProxyType	proxy_type	=	_proxy_config._proxy_type;

	if (proxy_type == PROXY_NULL)
	{
		return _socket.connect(_host_name,_port_number);
	}
	else
	{
		std::string	host_name	=	_proxy_config._host_name;
		int	port_number			=	_proxy_config._port_number;
		bool connect_proxy		=	_socket.connect(host_name,port_number);
		
		if (!connect_proxy)
		{
			return false;
		}
		else
		{
			bool connect_host	=	false;
			switch(proxy_type)
			{
			case PROXY_HTTP:
				connect_host	=	handShakeWithHttpProxy();
				break;
			case PROXY_SOCK4:
				connect_host	=	handShakeWithSock4Proxy();
				break;
			case PROXY_SOCK5:
				connect_host	=	handShakeWithSock5Proxy();
				break;
			default:
				assert(false);
				break;
			}
			return connect_host;
		}
	}
}


void	ProxySocket::close()
{
	_socket.close();
}

int		ProxySocket::read(char *dst_buffer, int max_read_length)
{
	return _socket.read(dst_buffer,max_read_length);
}

int		ProxySocket::write(const char *source_buffer, int max_write_length)
{
	return _socket.write(source_buffer,max_write_length);
}

bool	ProxySocket::writeAll(const char *source_buffer, int max_write_length)
{
	return _socket.writeAll(source_buffer,max_write_length);
}


bool	ProxySocket::handShakeWithHttpProxy()
{
	char buff[kmax_file_buffer_size] = {0};

	if (!_proxy_config._username.empty())
	{
		std::string auth = _proxy_config._username + ":" + _proxy_config._password;
		std::string base64_encode_auth;
		Util::base64Encode(auth,base64_encode_auth);
		sprintf_s(buff,kmax_file_buffer_size,"CONNECT %s:%d HTTP/1.1\r\nHost: %s:%d\r\nAuthorization: Basic %s\r\nProxy-Authorization: Basic %s\r\n\r\n",
				_host_name.c_str(),_port_number,_host_name.c_str(),_port_number,base64_encode_auth.c_str(),base64_encode_auth.c_str());
	}
	else
	{
		sprintf_s(buff,kmax_file_buffer_size,"CONNECT %s:%d HTTP/1.1\r\nHost: %s:%d\r\n\r\n",
					_host_name.c_str(),_port_number,_host_name.c_str(),_port_number);
	}

	//����HTTP������������
	bool send_connect_request = _socket.writeAll(buff,strlen(buff));
	if (!send_connect_request)
	{
		return false;
	}
	//���HTTP����ظ�
	int ret = _socket.read(buff,sizeof(buff));
	if (ret <= 0)
	{
		return false;
	}
	buff[ret] = '\0';

	Util::makeLower(buff,strlen(buff));
	return strstr(buff, "200 connection established") != 0;
}

bool	ProxySocket::handShakeWithSock4Proxy()
{
	//Socks4û���û�������֤
	struct Sock4Reqeust 
	{ 
		char VN; 
		char CD; 
		unsigned short port; 
		unsigned long ip_address; 
		char other[256]; // �䳤
	} sock4_request; 

	struct Sock4Reply 
	{ 
		char VN; 
		char CD; 
		unsigned short port; 
		unsigned long ip_address; 
	} sock4_reply; 

	sock4_request.VN = 0x04; // VN��SOCK�汾��Ӧ����4��
	sock4_request.CD = 0x01; // CD��SOCK�������룬1��ʾCONNECT����2��ʾBIND����
	sock4_request.port= ntohs(_port_number);
	sock4_request.ip_address = SocketHelper::getIntAddress(_host_name.c_str());
	sock4_request.other[0] = '\0';

	if (sock4_request.ip_address == INADDR_NONE)
		return false;

	//����SOCKS4��������
	bool send_sock4_requst =  _socket.writeAll((char*)&sock4_request,9);          
	if (!send_sock4_requst)
	{
		return false;
	}
	//���Socks4����Ļظ�
	int ret = _socket.read((char *)&sock4_reply, sizeof(sock4_reply));
	if (ret <= 0)
	{			
		return false;
	}
	/*
	CD�Ǵ���������𸴣��м��ֿ��ܣ�
	90������õ�����
	91�����󱻾ܾ���ʧ�ܣ�
	92������SOCKS�������޷����ӵ��ͻ��˵�identd��һ����֤��ݵĽ��̣������󱻾ܾ���
	93�����ڿͻ��˳�����identd������û���ݲ�ͬ�����ӱ��ܾ���
	*/
	return sock4_reply.CD == 90;
}

bool	ProxySocket::handShakeWithSock5Proxy()
{
	char buff[kmax_file_buffer_size] = {0};

	//��һ��������֤�û�
	struct FirstSock5Request 
	{
		char version; 
		char method; 
		char methods[255];
	} first_sock5_request; 
	first_sock5_request.version		= 0x05;			// socks5
	first_sock5_request.method		= 0x02;			// ��֤��ʽ������
	first_sock5_request.methods[0]	= 0x00;			// NO AUTHENTICATION REQUIRED
	first_sock5_request.methods[1]	= 0x02;			// USERNAME/PASSWORD

	bool send_first_sock4_requst	= _socket.writeAll((char*)&first_sock5_request,4);          
	if (!send_first_sock4_requst)
	{
		return false;
	}

	// ��һ��Ӧ��
	struct FirstSock5Answer 
	{ 
		char version; 
		char method;
	} first_sock5_answer;
	int ret = _socket.read((char*)&first_sock5_answer, sizeof(first_sock5_answer));
	if (ret <= 0 || 
		first_sock5_answer.version != 5)
	{			
		return false;
	}

	// ��Ҫ�û�����������֤
	if (first_sock5_answer.method == 0x02)
	{
		struct AuthRequest 
		{ 
			char version; 
			char name_length; 
			char name[255];			// �䳤
			char password_length; 
			char password[255];		// �䳤
		};
		AuthRequest * auth_request = (AuthRequest *)buff;
		int auth_length = 0;
		auth_request->version = 0x01; 
		auth_length ++;
		auth_request->name_length = _proxy_config._username.length(); 
		auth_length ++;
		strcpy_s(auth_request->name,255,_proxy_config._username.c_str()); 
		auth_length += _proxy_config._username.length();
		char password_length = (char)_proxy_config._password.length();
		memcpy((char*)auth_request+auth_length, &password_length, 1); 
		auth_length ++;
		strcpy_s((char*)auth_request+auth_length,255, _proxy_config._password.c_str()); 
		auth_length += _proxy_config._password.length();

		bool send_auth_data = _socket.writeAll(buff, auth_length);
		if (!send_auth_data)
		{
			return false;
		}

		struct AuthAnswer 
		{ 
			char Ver; 
			char Status; 
		} auth_answer;

		ret = _socket.read((char*)&auth_answer, sizeof(auth_answer)); 
		if (ret <= 0 || auth_answer.Status != 0x00)
		{			
			return false;
		}
	}

	// �ڶ�����������Ŀ���ַ
	struct SecondSock5Request 
	{ 
		char version; 
		char command; 
		char reserved; 
		char address_type; 
		unsigned long	dest_address;
		unsigned short	dest_port;
	} second_sock5_request;
	second_sock5_request.version		= 0x05; // socks�汾
	second_sock5_request.command		= 0x01; // CONNECT X'01', BIND X'02', UDP ASSOCIATE X'03'
	second_sock5_request.reserved		= 0x00; // RESERVED
	second_sock5_request.address_type	= 0x01; // IP V4 address: X'01', DOMAINNAME: X'03', IP V6 address: X'04'
	second_sock5_request.dest_address	= SocketHelper::getIntAddress(_host_name.c_str());
	second_sock5_request.dest_port		= ntohs(_port_number);

	if (second_sock5_request.dest_address == INADDR_NONE)
		return false;

	bool send_second_sock5_request = _socket.writeAll((char*)&second_sock5_request, 10); 
	if (!send_second_sock5_request)
	{
		return false;
	}

	// �ڶ���Ӧ��
	struct SecondSock5Answer 
	{ 
		char version; 
		char reply;
		char reserved; 
		char address_type; 
		char other[256];
	} second_sock5_answer;
	memset(&second_sock5_answer, 0, sizeof(second_sock5_answer));
	ret = _socket.read((char*)&second_sock5_answer, sizeof(second_sock5_answer));
	if (ret <= 0)
	{			
		return false;	
	}

	return second_sock5_answer.reply == 0x00; // X'00' succeeded
}


NAMESPACE_END(Http)
