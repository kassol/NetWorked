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

MyMsgProtco::MsgType MyMsgProtco::GetMsgType(char *szMsg)
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
	char *temMsg = new char[nstrlen];
	switch(mt)
	{
	case MT_CONNECT:
		nType = 0;
		_snprintf_s(temMsg, nstrlen, nstrlen, "%04X%d%s", nstrlen, nType, szMsg);
		break;
	case MT_COMMAND:
		nType = 1;
		_snprintf_s(temMsg, nstrlen, nstrlen, "%04X%d%s", nstrlen, nType, szMsg);
		break;
	case MT_FEEDBACK:
		nType = 3;
		_snprintf_s(temMsg, nstrlen, nstrlen, "%04X%d%s", nstrlen, nType, szMsg);
		break;
	case MT_HEARTBEAT:
		nType = 4;
		_snprintf_s(temMsg, nstrlen, nstrlen, "%04X%d%s", nstrlen, nType, szMsg);
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
	for (int i = 0; i < nstrlen; i ++)
	{
		tempMsg[i] = szMsg[i+1];
	}
	return tempMsg;
}