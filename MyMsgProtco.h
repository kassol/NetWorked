#pragma once

enum MsgType{MT_MASTER, MT_PING, MT_COMMAND, MT_FEEDBACK, MT_ERROR};

class MyMsgProtco
{
public:
	MyMsgProtco(void);
	~MyMsgProtco(void);

	static MsgType GetMsgType(char *szMsg);
	static char* EncodeMsg(MsgType mt, char *szMsg);
	static char* DecodeMsg(char *szMsg);
};

