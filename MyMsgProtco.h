#pragma once
class MyMsgProtco
{
public:
	MyMsgProtco(void);
	~MyMsgProtco(void);

	//Message Type
	enum MsgType{MT_CONNECT, MT_MASTER, MT_COMMAND, MT_FEEDBACK, MT_HEARTBEAT, MT_ERROR};
	MsgType GetMsgType(char *szMsg);
	char* EncodeMsg(MsgType mt, char *szMsg);
	char* DecodeMsg(char *szMsg);
private:
	char *m_szMsg;
};

