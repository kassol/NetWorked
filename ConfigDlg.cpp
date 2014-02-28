// ConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NetWorked.h"
#include "ConfigDlg.h"
#include "afxdialogex.h"
#include "NetWorkedDlg.h"


// CConfigDlg dialog

IMPLEMENT_DYNAMIC(CConfigDlg, CDialogEx)

CConfigDlg::CConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CConfigDlg::IDD, pParent)
	, m_nPort(8991)
	, m_nPckSize(1500)
{

}

CConfigDlg::~CConfigDlg()
{
}

void CConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PORT, m_nPort);
	DDX_Text(pDX, IDC_PCKSIZE, m_nPckSize);
}


BEGIN_MESSAGE_MAP(CConfigDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CConfigDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CConfigDlg message handlers


void CConfigDlg::OnBnClickedOk()
{
	UpdateData(TRUE);

	CNetWorkedDlg::SetPort(m_nPort);
	CNetWorkedDlg::SetPackageSize(m_nPckSize);
	
	CDialogEx::OnOK();
}
