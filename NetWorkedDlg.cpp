
// NetWorkedDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "NetWorked.h"
#include "NetWorkedDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNetWorkedDlg �Ի���

int CNetWorkedDlg::_nPort = 8991;
int CNetWorkedDlg::_nPackageSize = 1500;

void run_service(boost::asio::io_service& service);
void scanlog(CNetWorkedDlg* pdlg);
void start_work(CNetWorkedDlg* pdlg);

CNetWorkedDlg::CNetWorkedDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNetWorkedDlg::IDD, pParent)
	, m_pImage(NULL)
	, m_pConfDlg(NULL)
	, pNode(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	HRESULT hr = CoCreateInstance(CLSID_ImageDriverX, NULL, CLSCTX_ALL, IID_IImageX, (void**)&m_pImage);
	if (FAILED(hr))
	{
		AfxMessageBox(_T("Regist com library failed!"));
	}
	m_fs.open("log.txt", ios::out);
	pNode = new CNode(service, _nPort);
	boost::thread thrd(boost::bind(run_service, boost::ref(service)));
}

void run_service(boost::asio::io_service& service)
{
	service.run();
}

CNetWorkedDlg::~CNetWorkedDlg()
{
	m_fs.close();
	service.stop();
	delete pNode;
	pNode = NULL;
	if (m_pImage != NULL)
	{
		m_pImage->Release();
		m_pImage = NULL;
	}
	if (m_pConfDlg != NULL)
	{
		m_pConfDlg->DestroyWindow();
		delete m_pConfDlg;
		m_pConfDlg = NULL;
	}
}

void CNetWorkedDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_ctrlProgress);
	DDX_Control(pDX, IDC_LIST1, m_ctrlMsgList);
}

BEGIN_MESSAGE_MAP(CNetWorkedDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CNetWorkedDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CNetWorkedDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_EXIT, &CNetWorkedDlg::OnBnClickedExit)
	ON_BN_CLICKED(IDC_START, &CNetWorkedDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_CLEAR, &CNetWorkedDlg::OnBnClickedClear)
	ON_BN_CLICKED(IDC_CONFIG, &CNetWorkedDlg::OnBnClickedConfig)
END_MESSAGE_MAP()


// CNetWorkedDlg ��Ϣ�������

BOOL CNetWorkedDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	boost::thread thr(boost::bind(scanlog, this));

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CNetWorkedDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CNetWorkedDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CNetWorkedDlg::OnBnClickedOk()
{

}


void CNetWorkedDlg::OnBnClickedCancel()
{
	PostQuitMessage(0);
}


void CNetWorkedDlg::OnBnClickedExit()
{
	PostQuitMessage(0);
}

void scanlog(CNetWorkedDlg* pdlg)
{
	while(true)
	{
		std::deque<string> &log_list = pdlg->pNode->GetLogList();
		if (log_list.empty())
		{
			Sleep(100);
			continue;
		}
		else
		{
			CString strMsg(log_list.front().c_str());
			log_list.pop_front();
			pdlg->AddMsg(strMsg);
			pdlg->Log(strMsg);
		}
	}
}


void start_work(CNetWorkedDlg* pdlg)
{
	pdlg->pNode->Start();

// 	CString strMsg = _T("");
// 
// 	if (pdlg->pNode->IsConnected() && !pdlg->pNode->IsMaster())
// 	{
// 		strMsg = _T("����IP��")+CString(pdlg->pNode->GetIP().c_str());
// 		pdlg->AddMsg(strMsg);
// 
// 		if(pdlg->pNode->InCharge())
// 		{
// 			strMsg = _T("���������ڵ㣬ɨ��ڵ��У����Ժ�...");
// 			pdlg->AddMsg(strMsg);
// 			pdlg->pNode->ScanNode();
// 
// 			Sleep(5000);
// 
// 			std::vector<string> &ip_list = pdlg->pNode->GetIPList();
// 			if (!ip_list.empty())
// 			{
// 				for_each(ip_list.begin(), ip_list.end(), [&pdlg, &strMsg] (string ip_)
// 				{
// 					strMsg = _T("�������� ")+CString(ip_.c_str());
// 					pdlg->AddMsg(strMsg);
// 
// 				});
// 
// 
// 				strMsg = _T("ɨ����ϣ���ʼ�ַ�����");
// 				pdlg->AddMsg(strMsg);
// 				pdlg->pNode->Start();
// 			}
// 			else
// 			{
// 				strMsg = _T("δɨ�赽�ڵ�");
// 				pdlg->AddMsg(strMsg);
// 			}
// 		}
// 		else
// 		{
// 			strMsg = _T("���������ڵ�ʧ��");
// 			pdlg->AddMsg(strMsg);
// 		}
// 	}
// 	else if (!pdlg->pNode->IsConnected())
// 	{
// 		strMsg = _T("��������Чip");
// 		pdlg->AddMsg(strMsg);
// 	}
// 	else
// 	{
// 		strMsg = _T("�������ڵ�");
// 		pdlg->AddMsg(strMsg);
// 		std::vector<string> &ip_list = pdlg->pNode->GetIPList();
// 		if (ip_list.empty())
// 		{
// 			strMsg = _T("ɨ�������У����Ժ�...");
// 			pdlg->AddMsg(strMsg);
// 			pdlg->pNode->ScanNode();
// 
// 			Sleep(5000);
// 
// 			if (!ip_list.empty())
// 			{
// 				for_each(ip_list.begin(), ip_list.end(), [&pdlg, &strMsg] (string ip_)
// 				{
// 					strMsg = _T("�������� ")+CString(ip_.c_str());
// 					pdlg->AddMsg(strMsg);
// 
// 				});
// 
// 				strMsg = _T("�����������ڵ㣬��ʼ�ַ�����");
// 				pdlg->AddMsg(strMsg);
// 				pdlg->pNode->Start();
// 			}
// 			else
// 			{
// 				strMsg = _T("δɨ�赽����");
// 				pdlg->AddMsg(strMsg);
// 			}
// 		}
// 	}
}

void CNetWorkedDlg::OnBnClickedStart()
{
	boost::thread thr(boost::bind(start_work, this));
}

void CNetWorkedDlg::OnBnClickedClear()
{
	m_ctrlMsgList.ResetContent();
}


void CNetWorkedDlg::OnBnClickedConfig()
{
	if (m_pConfDlg == NULL)
	{
		m_pConfDlg = new CConfigDlg;
		m_pConfDlg->Create(IDD_CONFIG,this);
	}
	m_pConfDlg->ShowWindow(SW_SHOW);
}

void CNetWorkedDlg::SetPort(int nPort)
{
	_nPort = nPort;
}

void CNetWorkedDlg::SetPackageSize(int nPackageSize)
{
	_nPackageSize = nPackageSize;
}

int CNetWorkedDlg::GetPort()
{
	return _nPort;
}

int CNetWorkedDlg::GetPackageSize()
{
	return _nPackageSize;
}

void CNetWorkedDlg::AddMsg(CString strMsg)
{
	CTime tm = CTime::GetCurrentTime();
	CString str = tm.Format("[%Y/%m/%d %H:%M:%S] ");
	str += strMsg;
	m_ctrlMsgList.InsertString(m_ctrlMsgList.GetCount(), str);
	m_ctrlMsgList.SetTopIndex(max(m_ctrlMsgList.GetCount()-10, 0));
}

void CNetWorkedDlg::Log(CString strMsg)
{
	CTime tm = CTime::GetCurrentTime();
	CString str = tm.Format("[%Y/%m/%d %H:%M:%S] ");
	str += strMsg;
	USES_CONVERSION;
	m_fs<<T2A(str)<<endl;
}

bool CNetWorkedDlg::IsMaster()
{
	return pNode->IsMaster();
}