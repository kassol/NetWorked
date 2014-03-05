#pragma once
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread.hpp>
#include <boost/smart_ptr.hpp>
#include "MyMsgProtco.h"
#include <deque>
#include <fstream>

using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;  


class CNode : public boost::enable_shared_from_this<CNode>
	, boost::noncopyable
{
public:
	typedef boost::shared_ptr<CNode>nodeptr;
	enum NodeType{NT_MASTER, NT_NORMAL};
	friend class session;
public:
	CNode(boost::asio::io_service& io_service, short port)
		: nt_(NT_NORMAL)
		, ip_("")
		, next_ip("")
		, master_ip("")
		, io_service_(io_service)
		, listen_port(port)
		, acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
	{
		is_connected = Initialize();
		if (is_connected)
		{
			start_accept();
		}

	}
	~CNode(void){}

private:
	bool Initialize();
	void AddNodes();

private:
	void start_accept();
	void start_scan();
	void handle_accept(session* new_session, const boost::system::error_code& error);
	void handle_connect(session* new_session, const boost::system::error_code& error);
	void handle_msg(string ip, const char* msg);

public:
	bool IsConnected();
	bool IsMaster();
	bool InCharge();
	NodeType GetNodeType();
	string& GetIP();
	void ScanNode();
	void Start();
	void AddLog(string log);
	std::vector<string>& GetIPList();
	std::deque<string>& GetLogList();
	
private:
	NodeType nt_;
	string ip_;
	string next_ip;
	string master_ip;
	int listen_port;
	bool is_connected;
	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;
	std::vector<string> ip_list;
	std::deque<string> log_list;
	ofstream outfile;
};


class session
{
public:
	session(boost::asio::io_service& io_service, CNode* owner)
		: socket_(io_service)
		, owner_(owner)
	{
	}

	tcp::socket& socket()
	{
		return socket_;
	}

	void start()
	{
		boost::asio::async_read(socket_,
			boost::asio::buffer(data_in, 4),
			boost::asio::transfer_exactly(4),
			boost::bind(&session::handle_header, this, boost::asio::placeholders::error));
	}


	void send_msg(MsgType mt, const char* szbuf)
	{
		char* szresult = MyMsgProtco::EncodeMsg(mt, szbuf);
		memcpy(data_out, szresult, strlen(szresult)+1);
		delete []szresult;
		szresult = NULL;
		boost::asio::async_write(socket_,
			boost::asio::buffer(data_out, strlen(data_out)+1),
			boost::bind(&session::read_head, this,
			boost::asio::placeholders::error));
	}



private:

	void read_head(const boost::system::error_code& error)
	{
		if (!error)
		{
			boost::asio::async_read(socket_,
				boost::asio::buffer(data_in, 4),
				boost::asio::transfer_exactly(4),
				boost::bind(&session::handle_header, this,
				boost::asio::placeholders::error));
		}
		else
		{
			delete this;
		}
	}

	void handle_header(const boost::system::error_code& error)
	{
		if (!error)
		{
			char* stopstring;
			int bytes_to_transfer = strtol(data_in, &stopstring, 16);
			read_content(bytes_to_transfer-3, error);
		}
		else
		{
			delete this;
		}
	}

	void read_content(size_t bytes_to_transfer, const boost::system::error_code& error)
	{
		if (!error)
		{
			if (bytes_to_transfer > max_length)
			{
				boost::asio::async_read(socket_,
					boost::asio::buffer(data_in, max_length),
					boost::asio::transfer_exactly(max_length),
					boost::bind(&session::read_content, this,
					bytes_to_transfer-max_length,
					boost::asio::placeholders::error));
			}
			else
			{
				boost::asio::async_read(socket_,
					boost::asio::buffer(data_in, bytes_to_transfer),
					boost::asio::transfer_exactly(bytes_to_transfer),
					boost::bind(&session::handle_read, this,
					boost::asio::placeholders::error));
			}
		}
		else
		{
			delete this;
		}
	}

	void handle_read(const boost::system::error_code& error)
	{
		if (!error)
		{
			owner_->handle_msg(socket_.remote_endpoint().address().to_string(), data_in);

		}
		delete this;
	}

private:
	tcp::socket socket_;
	enum { max_length = 1024 };
	char data_out[1024];
	char data_in[1024];
	CNode* owner_;
};

