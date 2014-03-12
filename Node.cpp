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
	session* new_session = NULL;
	for_each(ip_list.begin(), ip_list.end(), [&](string ip)
	{
		new_session = new session(io_service_, this);
		new_session->socket().async_connect(tcp::endpoint(boost::asio::ip::address::from_string(ip), listen_port),
			boost::bind(&CNode::handle_connect_msg, this, new_session, new msg_struct(MT_MASTER, ""), boost::asio::placeholders::error));
	});
}

void CNode::ParseProj()
{	
// 	task_list.push_back(task_struct("D:\\data\\ads\\1\\J_L112040403PANB14A_0_0.tif", 0));
// 	task_list.push_back(task_struct("D:\\data\\ads\\1\\J_L112040403PANB14A_0_1.tif", 0));
	task_list.push_back(task_struct("C:\\outImg.tiff", 0));
}

void CNode::Distribute()
{
	while(available_list.empty())
	{
		Sleep(2000);
	}
	session* new_session = NULL;
	string strmsg;
	size_t i = 0;

	for_each(task_list.begin(), task_list.end(), [&](task_struct& task)
	{
		char filesize[50] = "";
		unsigned __int64 nfilesize = boost::filesystem::file_size(task.task_);
		_snprintf_s(filesize, 50, 50, "%I64x", nfilesize);
		string filename = task.task_.substr(task.task_.rfind('\\')+1, task.task_.size()-task.task_.rfind('\\')-1);
		strmsg = filesize+string("|");
		strmsg += filename;
		while (i < available_list.size())
		{
			if (!available_list.at(i).is_busy)
			{
				new_session = new session(io_service_, this);
				task.state_ = 1;
				task.ip_ = available_list.at(i).ip_;
				new_session->socket().async_connect(
					tcp::endpoint(boost::asio::ip::address::from_string(available_list.at(i).ip_), listen_port),
					boost::bind(&CNode::handle_connect_msg, this, new_session, 
					new msg_struct(MT_METAFILE,strmsg, available_list.at(i).ip_), boost::asio::placeholders::error));
				i = (i+1)%available_list.size();
				break;
			}
			i = (i+1)%available_list.size();
		}
	});
}

void CNode::Work()
{
	is_busy = true;
}

void CNode::AddLog(string log)
{
	log_list.push_back(log);
}

CNode::NodeType CNode::GetNodeType()
{
	return nt_;
}

int CNode::GetNodeNum()
{
	return available_list.size();
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
	
	AddLog("扫描节点中，请稍候...");

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

	AddLog("添加从节点中，请稍候...");

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

// 	if (!is_ping_busy)
// 	{
// 		boost::thread thrd(boost::bind(&CNode::start_ping, this));
// 	}

	Sleep(2000);
	
	AddLog("解析任务中，请稍候...");

	ParseProj();

	Sleep(2000);

	AddLog("分配任务中，请稍候...");

	Distribute();

	AddLog("任务分配完成");
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

void CNode::start_ping()
{
	is_ping_busy = true;
	session* new_session = NULL;
	while(is_ping_busy)
	{
		while (!available_list.empty())
		{
			for_each(available_list.begin(), available_list.end(), [&](node_struct node)
			{
				new_session = new session(io_service_, this);
				new_session->socket().async_connect(tcp::endpoint(boost::asio::ip::address::from_string(node.ip_), listen_port),
					boost::bind(&CNode::handle_connect_msg, this, new_session, new msg_struct(MT_PING, "", node.ip_), boost::asio::placeholders::error));
			});
			Sleep(5000);
		}
	}
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

void CNode::handle_accept_file(session* new_session, file_struct* file, const boost::system::error_code& error)
{
	if (!error)
	{
		new_session->recv_file(file->filename_, file->filesize_);
		delete file;
	}
	else
	{
		AddLog("监听失败");
		AddLog(error.message());

		delete new_session;
		delete file;
	}
}

void CNode::send_metafile(session* new_session, addr_struct* addr, const boost::system::error_code& error)
{
	if (!error)
	{
		AddLog("连接成功");
		string task_path;
		for (size_t i = 0; i < task_list.size(); ++i)
		{
			if (task_list.at(i).ip_ == addr->ip_)
			{
				task_path = task_list.at(i).task_;
				break;
			}
		}
		if (task_path == "")
		{
			AddLog("没有对应任务");
			delete new_session;
			delete addr;
			return;
		}
		AddLog("找到任务，开始发送文件");
		new_session->send_file(task_path, boost::filesystem::file_size(task_path));
		delete addr;
	}
	else
	{
		AddLog("连接失败");
		AddLog(error.message());
		delete new_session;
		new_session = new session(io_service_, this);
		new_session->socket().async_connect(tcp::endpoint(boost::asio::ip::address::from_string(addr->ip_), addr->port_),
			boost::bind(&CNode::send_metafile, this, new_session, addr, boost::asio::placeholders::error));
	}
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

void CNode::handle_connect_msg(session* new_session, msg_struct* msg, const boost::system::error_code& error)
{
	if (!error)
	{
		new_session->send_msg(msg->mt_, msg->msg_.c_str());
		delete msg;
	}
	else
	{
		if (msg->mt_ == MT_PING)
		{
			std::vector<node_struct>::iterator ite = std::find(available_list.begin(),
				available_list.end(), node_struct(msg->ip_));
			if (ite != available_list.end())
			{
				available_list.erase(ite);
				AddLog("丢失到"+msg->ip_+"的连接");
			}
		}
		delete new_session;
		delete msg;
	}
}

void CNode::handle_msg(string ip, const char* msg)
{
	MsgType etype = MyMsgProtco::GetMsgType(msg);
	char* szresult = MyMsgProtco::DecodeMsg(msg);
	static int i = 0;
	std::vector<node_struct>::iterator ite;
	session* new_session = NULL;
	switch(etype)
	{
	case MT_MASTER:
		{
			if (IsMaster())
			{
				new_session = new session(io_service_, this);
				new_session->socket().async_connect(tcp::endpoint(boost::asio::ip::address::from_string(ip), listen_port),
					boost::bind(&CNode::handle_connect_msg, this, new_session, new msg_struct(MT_FAIL, ip_), boost::asio::placeholders::error));
			}
			else
			{
				if (master_ip == "")
				{
					master_ip = ip;
					AddLog("已连接主节点"+master_ip);
					session* new_session = new session(io_service_, this);
					new_session->socket().async_connect(tcp::endpoint(boost::asio::ip::address::from_string(ip), listen_port),
						boost::bind(&CNode::handle_connect_msg, this, new_session, new msg_struct(MT_SUCCESS, ""), boost::asio::placeholders::error));
				}
				else
				{
					session* new_session = new session(io_service_, this);
					new_session->socket().async_connect(tcp::endpoint(boost::asio::ip::address::from_string(ip), listen_port),
						boost::bind(&CNode::handle_connect_msg, this, new_session, new msg_struct(MT_FAIL, master_ip), boost::asio::placeholders::error));
				}
			}
			break;
		}
	case MT_SUCCESS:
		{
			ite = std::find(available_list.begin(), available_list.end(), node_struct(ip));
			if (ite == available_list.end())
			{
				available_list.push_back(node_struct(ip));
				AddLog("添加从节点"+ip);
			}
			break;
		}
	case MT_FAIL:
		{
			if (ip_ == string(szresult))
			{
				AddLog("节点"+ip+"已是本机从节点");
			}
			else
			{
				AddLog("节点"+ip+"无法成为本机从节点");
			}
			break;
		}
	case MT_METAFILE:
		{
			char* pathname;
			unsigned __int64 filesize = _strtoui64(szresult, &pathname, 16);
			AddLog("接收文件"+string(pathname).substr(1, string(pathname).size()-1));
			unsigned short file_port = 8999;
			AddLog("开始绑定端口");
			boost::system::error_code er;
			boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), file_port);
			file_acceptor_.open(endpoint.protocol());
			file_acceptor_.bind(endpoint, er);
			file_acceptor_.listen();
			if (er)
			{
				AddLog(er.message());
				break;
			}
			AddLog("绑定端口成功");
			new_session = new session(io_service_, this);
			file_acceptor_.async_accept(new_session->socket(),
				boost::bind(&CNode::handle_accept_file, this, new_session, new file_struct(pathname, filesize), boost::asio::placeholders::error));
			AddLog("开启监听8999端口");
			new_session = new session(io_service_, this);
			char* pfileport = new char[5];
			memset(pfileport, 0, 5);
			_snprintf_s(pfileport, 5, 5, "%04X", file_port);
			new_session->socket().async_connect(tcp::endpoint(boost::asio::ip::address::from_string(ip), listen_port),
				boost::bind(&CNode::handle_connect_msg, this, new_session, new msg_struct(MT_METAFILE_READY, pfileport), boost::asio::placeholders::error));
			break;
		}
	case MT_METAFILE_READY:
		{
			char* rest;
			unsigned short file_port = (unsigned short)strtoul(szresult, &rest, 16);
			new_session = new session(io_service_, this);
			new_session->socket().async_connect(tcp::endpoint(boost::asio::ip::address::from_string(ip), file_port),
				boost::bind(&CNode::send_metafile, this, new_session, new addr_struct(ip, file_port), boost::asio::placeholders::error));
			break;
		}
	case MT_METAFILE_FAIL:
		{
			break;
		}
	case MT_FILE:
		{
			break;
		}
	case MT_FILE_READY:
		{
			break;
		}
	case MT_FILE_FAIL:
		{
			break;
		}
	case MT_COMMAND:
		{
			break;
		}
	case MT_FEEDBACK:
		{
			AddLog(string(szresult));
			break;
		}
	case MT_PING:
		{
			new_session = new session(io_service_, this);
			new_session->socket().async_connect(tcp::endpoint(boost::asio::ip::address::from_string(ip), listen_port),
				boost::bind(&CNode::handle_connect_msg, this, new_session, new msg_struct(MT_PINGBACK, is_busy?"-1":"1", ip), boost::asio::placeholders::error));
			break;
		}
	case MT_PINGBACK:
		{
			ite = std::find(available_list.begin(), available_list.end(), node_struct(ip));
			if (ite != available_list.end())
			{
				if (szresult == "-1")
				{
					ite->is_busy = true;
				}
				else if (szresult == "1")
				{
					ite->is_busy = false;
				}
			}
			break;
		}
	default:
		break;
	}
	delete []szresult;
	szresult = NULL;
}
