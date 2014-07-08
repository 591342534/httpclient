/**
 * @file    http_global.h
 * @brief   Httpȫ�����ݶ���
 * @author  xiangwangfeng <xiangwangfeng@gmail.com>
 * @data	2011-4-24
 * @website www.xiangwangfeng.com
 */
#pragma once
#include "global_defs.h"

const	char	kget[]				=	"GET";
const	char	kpost[]				=	"POST";
const	char	kaccept[]			=	"Accept";
const	char	kconnection[]		=	"Connection";
const	char	kcontent_type[]		=	"Content-Type";
const	char	kcontent_length[]	=	"Content-Length";

NAMESPACE_BEGIN(Http)

//Http���������
enum	HTTPERROR
{
	HTTPERROR_SUCCESS,		//��ȷ
	HTTPERROR_INVALID,		//HTTP�Ѿ�������
	HTTPERROR_CONNECT,		//���ӳ���
	HTTPERROR_TRANSPORT,	//����ʧ��
	HTTPERROR_IO,			//IO����
	HTTPERROR_PARAMETER		//��Ч����
};

NAMESPACE_END(Http)