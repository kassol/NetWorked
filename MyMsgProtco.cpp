#include "StdAfx.h"
#include "MyMsgProtco.h"
#include <intrin.h>
#include <string.h>


MyMsgProtco::MyMsgProtco(void)
{
}


MyMsgProtco::~MyMsgProtco(void)
{
}

MsgType MyMsgProtco::GetMsgType(const char *szMsg)
{
	char* endptr;
	char* p = new char[2];
	memset(p, 0, 2);
	memcpy(p, szMsg, 2);
	int temp = strtol(p, &endptr, 16);

	if (temp < MT_ERROR)
	{
		return MsgType(temp);
	}

	return MT_ERROR;
}

char* MyMsgProtco::EncodeMsg(MsgType mt, const char *szMsg)
{
	int nstrlen = strlen(szMsg)+2+4;
	char *temMsg = new char[nstrlen+1];
	_snprintf_s(temMsg, nstrlen+1, nstrlen+1, "%04X%02X%s", nstrlen, mt, szMsg);
	return temMsg;
}

char* MyMsgProtco::DecodeMsg(const char *szMsg)
{
	int nstrlen = strlen(szMsg);
	char *tempMsg = new char[nstrlen];
	memcpy(tempMsg, szMsg+2, nstrlen);
	return tempMsg;
}