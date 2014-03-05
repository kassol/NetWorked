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
				if (IDYES == AfxMessageBox(_T("È·ÈÏipÎª")+CString(ip.c_str())+_T("?"), MB_YESNO))
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

bool CNode::IsMaster()
{
	if (nt_ == NT_MASTER)
	{
		return true;
	}
	else
	{
		return false;
	}
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

}

void CNode::ScanNode()
{
	start_scan();
}

void CNode::start_accept()
{
	session* new_session = new session(io_service_, this);
	acceptor_.async_accept(new_session->socket(),
		boost::bind(&CNode::handle_accept, this, new_session, boost::asio::placeholders::error));
}

void CNode::start_scan()
{
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
		string str_ip = abc+_itoa(newnd, tmp, 10);
		boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(str_ip), listen_port);
		boost::system::error_code error;
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
			ip_list.push_back(ip);
			new_session->send_msg(MT_MASTER, const_cast<char*>(ip.c_str()));
		}
	}
	else
	{
		delete new_session;
	}
}

