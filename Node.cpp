#include "StdAfx.h"
#include "Node.h"


bool CNode::Initialize()
{
	try
	{
		tcp::resolver resolver(io_service_);  
		tcp::resolver::query query(ip::host_name(), "");
		tcp::resolver::iterator iter = resolver.resolve(query);
		tcp::resolver::iterator end;
		while (iter != end)
		{
			tcp::endpoint ep = *iter++;
			string ip = ep.address().to_string();
			int index = ip.find('.');
			if (index == string::npos)
			{
				continue;
			}
			else
			{
				if (IDYES == AfxMessageBox(_T("确认ip为")+CString(ip.c_str())+_T("?"), MB_YESNO))
				{
					ip_ = ip;
					return true;
				}
				continue;
			}
		}  
	}
	catch(std::exception& e)
	{
		AfxMessageBox(CString(e.what()));
		return false;
	}
	return false;
}

bool CNode::IsConnected()
{
	return is_connected;
}

bool CNode::IsScanFinished()
{
	return is_scan_finished;
}

bool CNode::IsMaster()
{
	return nt_ == NT_MASTER;
}

bool CNode::InCharge()
{
	if (master_ip == "")
	{
		nt_ = NT_MASTER;
		master_ip = ip_;
		AddNodes();
		return true;
	}
	return false;
}

void CNode::AddNodes()
{
	available_list.clear();
	session* new_session = NULL;
	for_each(ip_list.begin(), ip_list.end(), [&](string ip)
	{
		new_session = new session(io_service_, this);
		new_session->socket().async_connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip), listen_port),
			boost::bind(&CNode::handle_connect_msg, this, new_session, MT_MASTER, "", boost::asio::placeholders::error));
	});
}

void CNode::AddLog(string log)
{
	log_list.push_back(log);
}

CNode::NodeType CNode::GetNodeType()
{
	return nt_;
}

string& CNode::GetIP()
{
	return ip_;
}

std::vector<string>& CNode::GetIPList()
{
	return ip_list;
}

std::deque<string>& CNode::GetLogList()
{
	return log_list;
}

void CNode::Start()
{
	if (!is_connected)
	{
		return;
	}

	if (!is_scan_finished)
	{
		AddLog("上次扫描未结束");
		return;
	}
	
	AddLog("扫描节点中，请稍后...");

	start_scan();

	Sleep(2000);

	if (ip_list.empty())
	{
		if (is_scan_finished)
		{
			AddLog("未扫描到上线节点");
		}
		return;
	}

	AddLog("添加从节点中，请稍后...");

	Sleep(2000);

	if (IsMaster())
	{
		AddNodes();
	}
	else
	{
		if (InCharge())
		{

		}
		else
		{
			AddLog("无法提升至主节点");
		}
	}

	Sleep(2000);


	
}

void CNode::start_accept()
{
	session* new_session = new session(io_service_, this);
	acceptor_.async_accept(new_session->socket(),
		boost::bind(&CNode::handle_accept, this, new_session, boost::asio::placeholders::error));
}

void CNode::start_scan()
{
	is_scan_finished = false;

	ip_list.clear();

	session* new_session = NULL;

	int index = ip_.rfind('.')+1;
	string d = ip_.substr(index, ip_.length()-index);
	int nd = atoi(d.c_str());
	string abc = ip_.substr(0, index);
	char* tmp = new char[10];
	
	for (int n = 1; n < 255; ++n)
	{
		new_session = new session(io_service_, this);
		int newnd = (nd+n)%255;
		_itoa_s(newnd, tmp, 10, 10);
		string str_ip = abc+tmp;
		new_session->socket().async_connect(tcp::endpoint(boost::asio::ip::address::from_string(str_ip), listen_port),
			boost::bind(&CNode::handle_connect, this, new_session, boost::asio::placeholders::error));
	}
	delete []tmp;
	tmp = NULL;
}

void CNode::handle_accept(session* new_session, const boost::system::error_code& error)
{
	if (!error)
	{
		new_session->start();
	}
	else
	{
		delete new_session;
	}
	start_accept();
}

void CNode::handle_connect(session* new_session, const boost::system::error_code& error)
{
	static int scan_count = 0;
	scan_count = (scan_count+1)%254;
	if (!error)
	{
		boost::system::error_code ec;
		boost::asio::ip::tcp::endpoint endpoint = new_session->socket().remote_endpoint(ec);
		if (ec)
		{
			delete new_session;
		}
		else
		{
			string ip = endpoint.address().to_string();
			std::vector<string>::iterator ite = std::find(ip_list.begin(), ip_list.end(), ip);
			if (ite == ip_list.end())
			{
				ip_list.push_back(ip);
			}
		}
	}
	else
	{
		delete new_session;
	}
	if (scan_count == 0)
	{
		is_scan_finished = true;
	}
}

void CNode::handle_connect_msg(session* new_session, MsgType mt, const char* szbuf, const boost::system::error_code& error)
{
	if (!error)
	{
		new_session->send_msg(mt, szbuf);
	}
	else
	{
		delete new_session;
	}
}

void CNode::handle_msg(string ip, const char* msg)
{
	MsgType etype = MyMsgProtco::GetMsgType(msg);
	char* szresult = MyMsgProtco::DecodeMsg(msg);
	static int i = 0;
	std::vector<string>::iterator ite;
	switch(etype)
	{
	case MT_MASTER:
		if (IsMaster())
		{
			session* new_session = new session(io_service_, this);
			new_session->socket().async_connect(tcp::endpoint(boost::asio::ip::address::from_string(ip), listen_port),
				boost::bind(&CNode::handle_connect_msg, this, new_session, MT_FAIL, "", boost::asio::placeholders::error));
		}
		else
		{
			if (master_ip == "")
			{
				master_ip = ip;
				AddLog("已连接主节点"+master_ip);
				session* new_session = new session(io_service_, this);
				new_session->socket().async_connect(tcp::endpoint(boost::asio::ip::address::from_string(ip), listen_port),
					boost::bind(&CNode::handle_connect_msg, this, new_session, MT_SUCCESS, "", boost::asio::placeholders::error));
			}
			else
			{
				session* new_session = new session(io_service_, this);
				new_session->socket().async_connect(tcp::endpoint(boost::asio::ip::address::from_string(ip), listen_port),
					boost::bind(&CNode::handle_connect_msg, this, new_session, MT_FAIL, master_ip.c_str(), boost::asio::placeholders::error));
			}
		}
		break;

	case MT_SUCCESS:
		ite = std::find(available_list.begin(), available_list.end(), ip);
		if (ite == available_list.end())
		{
			available_list.push_back(ip);
		}
		AddLog("添加从节点"+ip);
		break;
	case MT_FAIL:
		if (ip_ == string(szresult))
		{
			ite = std::find(available_list.begin(), available_list.end(), ip);
			if (ite == available_list.end())
			{
				available_list.push_back(ip);
			}
			AddLog("节点"+ip+"已是本机从节点");
		}
		else
		{
			AddLog("节点"+ip+"无法成为本机从节点");
		}
		break;
	case MT_METAFILE:
		break;
	case MT_METAFILE_READY:
		break;
	case MT_METAFILE_FAIL:
		break;
	case MT_FILE:
		break;
	case MT_FILE_READY:
		break;
	case MT_FILE_FAIL:
		break;
	case MT_COMMAND:
		break;
	case MT_FEEDBACK:
		AddLog(string(szresult));
		break;
	default:
		break;
	}
	delete []szresult;
	szresult = NULL;
}
