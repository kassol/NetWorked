
// NetWorked.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CNetWorkedApp:
// �йش����ʵ�֣������ NetWorked.cpp
//

class CNetWorkedApp : public CWinApp
{
public:
	CNetWorkedApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CNetWorkedApp theApp;