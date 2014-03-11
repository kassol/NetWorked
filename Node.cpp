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
				if (IDYES == AfxMessageBox(_T("ȷ��ipΪ")+CString(ip.c_str())+_T("?"), MB_YESNO))
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
		new_session->socket().async_connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip), listen_port),
			boost::bind(&CNode::handle_connect_msg, this, new_session, ip, MT_MASTER, "", boost::asio::placeholders::error));
	});
}

void CNode::ParseProj()
{
	task_struct task("C:\\test.tfw", 0);
	task_list.push_back(task);
}

void CNode::Distribute()
{
	session* new_session = NULL;
	string strmsg;
	size_t i = 0;
	for_each(task_list.begin(), task_list.end(), [&](task_struct task)
	{
		new_session = new session(io_service_, this);
		char filesize[50] = "";
		_snprintf_s(filesize, 50, 50, "%016X", boost::filesystem::file_size(task.task_));
		string filename = task.task_.substr(task.task_.rfind('\\')+1, task.task_.size()-task.task_.rfind('\\')-1);
		strmsg = filesize+filename;
		while (i < available_list.size())
		{
			if (!available_list.at(i).is_busy)
			{
				new_session = new session(io_service_, this);
				task.state_ = 1;
				task.ip_ = available_list.at(i).ip_;
				new_session->socket().async_connect(
					tcp::endpoint(boost::asio::ip::address::from_string(available_list.at(i).ip_), listen_port),
					boost::bind(&CNode::handle_connect_msg, this, new_session, available_list.at(i).ip_, 
					MT_METAFILE, strmsg.c_str(), boost::asio::placeholders::error));
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
		AddLog("�ϴ�ɨ��δ����");
		return;
	}
	
	AddLog("ɨ��ڵ��У����Ժ�...");

	start_scan();

	Sleep(2000);

	if (ip_list.empty())
	{
		if (is_scan_finished)
		{
			AddLog("δɨ�赽���߽ڵ�");
		}
		return;
	}

	AddLog("��Ӵӽڵ��У����Ժ�...");

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
			AddLog("�޷����������ڵ�");
		}
	}

	Sleep(2000);

// 	if (!is_ping_busy)
// 	{
// 		boost::thread thrd(boost::bind(&CNode::start_ping, this));
// 	}

	Sleep(2000);
	
	AddLog("���������У����Ժ�...");

	ParseProj();

	Sleep(2000);

	AddLog("���������У����Ժ�...");

	Distribute();

	AddLog("����������");
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
					boost::bind(&CNode::handle_connect_msg, this, new_session, node.ip_, MT_PING, "", boost::asio::placeholders::error));
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

void CNode::handle_accept_file(session* new_session, string filename, long filesize, const boost::system::error_code& error)
{
	if (!error)
	{
		AddLog("�ɹ�����");
		new_session->recv_file(filename, filesize);
	}
	else
	{
		AddLog("����ʧ��");
		delete new_session;

		
		unsigned short file_port = 8999;
		tcp::acceptor file_acceptor(io_service_, tcp::endpoint(tcp::v4(), file_port));
		new_session = new session(io_service_, this);
		file_acceptor.async_accept(new_session->socket(),
			boost::bind(&CNode::handle_accept_file, this, new_session, filename, filesize, boost::asio::placeholders::error));
		AddLog("��������8999�˿�");
	}
}

void CNode::send_metafile(session* new_session, string ip, unsigned short file_port, const boost::system::error_code& error)
{
	if (!error)
	{
		string task_path;
		for (size_t i = 0; i < task_list.size(); ++i)
		{
			if (task_list.at(i).ip_ == ip)
			{
				task_path == task_list.at(i).task_;
				break;
			}
		}
		if (task_path == "")
		{
			delete new_session;
			return;
		}
		new_session->send_file(task_path, (unsigned long)boost::filesystem::file_size(task_path));
	}
	else
	{
		AddLog("����ʧ��");
		delete new_session;
		new_session = new session(io_service_, this);
		new_session->socket().async_connect(tcp::endpoint(boost::asio::ip::address::from_string(ip), file_port),
			boost::bind(&CNode::send_metafile, this, new_session, ip, file_port, boost::asio::placeholders::error));
		AddLog("��������");
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

void CNode::handle_connect_msg(session* new_session, string ip, MsgType mt, const char* szbuf, const boost::system::error_code& error)
{
	if (!error)
	{
		new_session->send_msg(mt, szbuf);
	}
	else
	{
		if (mt == MT_PING)
		{
			std::vector<node_struct>::iterator ite = std::find(available_list.begin(),
				available_list.end(), node_struct(ip));
			if (ite != available_list.end())
			{
				available_list.erase(ite);
				AddLog("��ʧ��"+ip+"������");
			}
		}
		delete new_session;
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
					boost::bind(&CNode::handle_connect_msg, this, new_session, "", MT_FAIL, ip_.c_str(), boost::asio::placeholders::error));
			}
			else
			{
				if (master_ip == "")
				{
					master_ip = ip;
					AddLog("���������ڵ�"+master_ip);
					session* new_session = new session(io_service_, this);
					new_session->socket().async_connect(tcp::endpoint(boost::asio::ip::address::from_string(ip), listen_port),
						boost::bind(&CNode::handle_connect_msg, this, new_session, "", MT_SUCCESS, "", boost::asio::placeholders::error));
				}
				else
				{
					session* new_session = new session(io_service_, this);
					new_session->socket().async_connect(tcp::endpoint(boost::asio::ip::address::from_string(ip), listen_port),
						boost::bind(&CNode::handle_connect_msg, this, new_session, "", MT_FAIL, master_ip.c_str(), boost::asio::placeholders::error));
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
				AddLog("��Ӵӽڵ�"+ip);
			}
			break;
		}
	case MT_FAIL:
		{
			if (ip_ == string(szresult))
			{
				AddLog("�ڵ�"+ip+"���Ǳ����ӽڵ�");
			}
			else
			{
				AddLog("�ڵ�"+ip+"�޷���Ϊ�����ӽڵ�");
			}
			break;
		}
	case MT_METAFILE:
		{
			char* pathname;
			unsigned long filesize = strtoul(szresult, &pathname, 16);
			unsigned short file_port = 8999;
			tcp::acceptor file_acceptor(io_service_, tcp::endpoint(tcp::v4(), file_port));
			new_session = new session(io_service_, this);
			file_acceptor.async_accept(new_session->socket(),
				boost::bind(&CNode::handle_accept_file, this, new_session, pathname, filesize, boost::asio::placeholders::error));
			AddLog("��������8999�˿�");
			new_session = new session(io_service_, this);
			char* pfileport = new char[5];
			memset(pfileport, 0, 5);
			_snprintf_s(pfileport, 5, 5, "%04X", file_port);
			new_session->socket().async_connect(tcp::endpoint(boost::asio::ip::address::from_string(ip), listen_port),
				boost::bind(&CNode::handle_connect_msg, this, new_session, "", MT_METAFILE_READY, pfileport, boost::asio::placeholders::error));
			break;
		}
	case MT_METAFILE_READY:
		{
			char* rest;
			unsigned short file_port = (unsigned short)strtoul(szresult, &rest, 16);
			new_session = new session(io_service_, this);
			new_session->socket().async_connect(tcp::endpoint(boost::asio::ip::address::from_string(ip), file_port),
				boost::bind(&CNode::send_metafile, this, new_session, ip, file_port, boost::asio::placeholders::error));
			AddLog("���ӽڵ�");
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
				boost::bind(&CNode::handle_connect_msg, this, new_session, ip, MT_PINGBACK, is_busy ? "-1" : "1", boost::asio::placeholders::error));
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
