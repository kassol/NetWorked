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

MsgType MyMsgProtco::GetMsgType(char *szMsg)
{
	char temp = szMsg[0];
	if (temp>=48 && temp <= 57)
	{
		return MsgType(temp-48);
	}
	return MT_ERROR;
}

char* MyMsgProtco::EncodeMsg(MsgType mt, char *szMsg)
{
	int nType = 0;
	int nstrlen = strlen(szMsg)+2+4;
	char *temMsg = new char[nstrlen+1];
	switch(mt)
	{
	case MT_MASTER:
		nType = 0;
		_snprintf_s(temMsg, nstrlen+1, nstrlen+1, "%04X%02X%s", nstrlen, nType, szMsg);
		break;
	case MT_PING:
		nType = 1;
		_snprintf_s(temMsg, nstrlen+1, nstrlen+1, "%04X%02X%s", nstrlen, nType, szMsg);
		break;
	case MT_COMMAND:
		nType = 2;
		_snprintf_s(temMsg, nstrlen+1, nstrlen+1, "%04X%02X%s", nstrlen, nType, szMsg);
		break;
	case MT_FEEDBACK:
		nType = 3;
		_snprintf_s(temMsg, nstrlen+1, nstrlen+1, "%04X%02X%s", nstrlen, nType, szMsg);
		break;
	default:
		break;
	}
	return temMsg;
}

char* MyMsgProtco::DecodeMsg(char *szMsg)
{
	int nstrlen = strlen(szMsg);
	char *tempMsg = new char[nstrlen];
	memcpy(tempMsg, szMsg+2, nstrlen);
	return tempMsg;
}