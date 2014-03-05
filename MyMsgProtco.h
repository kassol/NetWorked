#pragma once

enum MsgType
{
	MT_MASTER, 
	MT_METAFILE, 
	MT_METAFILE_READY,
	MT_METAFILE_FAIL,
	MT_FILE,
	MT_FILE_READY,
	MT_FILE_FAIL,
	MT_COMMAND, 
	MT_FEEDBACK, 
	MT_ERROR
};

class MyMsgProtco
{
public:
	MyMsgProtco(void);
	~MyMsgProtco(void);

	static MsgType GetMsgType(const char *szMsg);
	static char* EncodeMsg(MsgType mt, const char *szMsg);
	static char* DecodeMsg(const char *szMsg);
};

