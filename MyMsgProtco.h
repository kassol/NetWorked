#pragma once

enum MsgType
{
	MT_MASTER,
	MT_SUCCESS,
	MT_FAIL,
	MT_METAFILE,
	MT_METAFILE_READY,
	MT_METAFILE_FINISH,
	MT_METAFILE_FAIL,
	MT_FILE_REQUEST,
	MT_FILE,
	MT_FILE_READY,
	MT_FILE_FINISH,
	MT_FILE_FAIL,
	MT_COMMAND,
	MT_FEEDBACK,
	MT_PING,
	MT_PINGBACK,
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

