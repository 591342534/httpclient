/**
 * @file    standard_header.h
 * @brief   ��׼ͷ�ļ�
 * @author  xiangwangfeng <xiangwangfeng@gmail.com>
 * @data	2011-7-23
 * @website www.xiangwangfeng.com
 */
#pragma once
#include <cassert>
#include <iostream>
#include <cstdio>
#include<algorithm>
#include <locale>
#include <cctype>
#include <sys/stat.h>
#include <WinSock2.h>
#include <Windows.h>

#define HTTP_CLIENT_EXPORT	//������ʶ

#define PTR_BOOL(p)		{assert(p); if(!p){return false;}}
#define PTR_VOID(p)		{assert(p); if(!p){return;}}
#define SAFE_DELETE(p)	{if(p) {delete p; p = 0;}}

const int	kmax_file_buffer_size		=	10240;	//�ϴ������ļ���buffer�Ĵ�С
const int	kmax_buffer_size			=	1024;	//��ͨ�����buffer��С
const int	kmax_http_pool_threads_num	=	5;
