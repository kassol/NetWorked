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
	char* p = new char[3];
	memset(p, 0, 3);
	memcpy(p, szMsg, 2);
	unsigned long temp = strtoul(p, &endptr, 16);

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
	int nstrlen = strlen(szMsg)-2;
	char *tempMsg = new char[nstrlen+1];
	memcpy(tempMsg, szMsg+2, nstrlen+1);
	return tempMsg;
}