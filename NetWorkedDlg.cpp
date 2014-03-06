
// NetWorkedDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "NetWorked.h"
#include "NetWorkedDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNetWorkedDlg 对话框

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


// CNetWorkedDlg 消息处理程序

BOOL CNetWorkedDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	boost::thread thr(boost::bind(scanlog, this));

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CNetWorkedDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
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
// 		strMsg = _T("本机IP是")+CString(pdlg->pNode->GetIP().c_str());
// 		pdlg->AddMsg(strMsg);
// 
// 		if(pdlg->pNode->InCharge())
// 		{
// 			strMsg = _T("提升至主节点，扫描节点中，请稍后...");
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
// 					strMsg = _T("已连接至 ")+CString(ip_.c_str());
// 					pdlg->AddMsg(strMsg);
// 
// 				});
// 
// 
// 				strMsg = _T("扫描完毕，开始分发任务");
// 				pdlg->AddMsg(strMsg);
// 				pdlg->pNode->Start();
// 			}
// 			else
// 			{
// 				strMsg = _T("未扫描到节点");
// 				pdlg->AddMsg(strMsg);
// 			}
// 		}
// 		else
// 		{
// 			strMsg = _T("提升至主节点失败");
// 			pdlg->AddMsg(strMsg);
// 		}
// 	}
// 	else if (!pdlg->pNode->IsConnected())
// 	{
// 		strMsg = _T("本机无有效ip");
// 		pdlg->AddMsg(strMsg);
// 	}
// 	else
// 	{
// 		strMsg = _T("已是主节点");
// 		pdlg->AddMsg(strMsg);
// 		std::vector<string> &ip_list = pdlg->pNode->GetIPList();
// 		if (ip_list.empty())
// 		{
// 			strMsg = _T("扫描主机中，请稍后...");
// 			pdlg->AddMsg(strMsg);
// 			pdlg->pNode->ScanNode();
// 
// 			Sleep(5000);
// 
// 			if (!ip_list.empty())
// 			{
// 				for_each(ip_list.begin(), ip_list.end(), [&pdlg, &strMsg] (string ip_)
// 				{
// 					strMsg = _T("已连接至 ")+CString(ip_.c_str());
// 					pdlg->AddMsg(strMsg);
// 
// 				});
// 
// 				strMsg = _T("已提升至主节点，开始分发任务");
// 				pdlg->AddMsg(strMsg);
// 				pdlg->pNode->Start();
// 			}
// 			else
// 			{
// 				strMsg = _T("未扫描到主机");
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