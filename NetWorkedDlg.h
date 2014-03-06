
// NetWorkedDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "ConfigDlg.h"
#include "Node.h"



// CNetWorkedDlg 对话框
class CNetWorkedDlg : public CDialogEx
{
// 构造
public:
	CNetWorkedDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CNetWorkedDlg();

	friend class CConfigDlg;

// 对话框数据
	enum { IDD = IDD_NETWORKED_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	IImageX* m_pImage;
	CConfigDlg* m_pConfDlg;
	fstream m_fs;
	static int _nPort;
	static int _nPackageSize;
	static void SetPort(int nPort);
	static void SetPackageSize(int nPackageSize);

public:
	static int GetPort();
	static int GetPackageSize();

public:
	void AddMsg(CString strMsg);
	void Log(CString strMsg);
	bool IsMaster();



public:
	CNode* pNode;
	boost::asio::io_service service;

public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedExit();
	CProgressCtrl m_ctrlProgress;
	CListBox m_ctrlMsgList;
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedClear();
	afx_msg void OnBnClickedConfig();
};

