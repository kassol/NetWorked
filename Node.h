#pragma once
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/filesystem.hpp>
#include "MyMsgProtco.h"
#include <deque>
#include <fstream>

using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;


struct task_struct 
{
	task_struct(string task, unsigned int state)
		: task_(task)
		, state_(state)
	{
		ip_ = "";
	}

	string task_;
	unsigned int state_;
	string ip_;
};

struct node_struct
{
	node_struct(string ip)
		: ip_(ip)
	{
		is_busy = false;
	}
	string ip_;
	bool is_busy;

	bool operator==(const node_struct& node)
	{
		return node.ip_==ip_;
	}
};

struct msg_struct
{
	msg_struct(MsgType mt, string msg, string ip)
		: mt_(mt)
		, msg_(msg)
		, ip_(ip)
	{

	}
	msg_struct(MsgType mt, string msg)
		: mt_(mt)
		, msg_(msg)
	{
		ip_ = "";
	}
	MsgType mt_;
	string msg_;
	string ip_;
};

struct addr_struct
{
	addr_struct(string ip, unsigned short port)
		: ip_(ip)
		, port_(port)
	{

	}
	string ip_;
	unsigned short port_;
};

struct file_struct
{
	file_struct(string filename, unsigned __int64 filesize)
		: filename_(filename)
		, filesize_(filesize)
	{

	}
	string filename_;
	unsigned __int64 filesize_;
};

class CNode : public boost::enable_shared_from_this<CNode>
	, boost::noncopyable
{
public:
	typedef boost::shared_ptr<CNode>nodeptr;
	enum NodeType{NT_MASTER, NT_NORMAL};
	friend class session;
public:
	CNode(boost::asio::io_service& io_service, unsigned short port)
		: nt_(NT_NORMAL)
		, ip_("")
		, next_ip("")
		, master_ip("")
		, io_service_(io_service)
		, listen_port(port)
		, acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
		, is_scan_finished(true)
		, is_ping_busy(false)
		, is_busy(false)
		, file_acceptor_(io_service)
		, limit_filenum_to_transfer(2)
		, cur_filenum(0)
	{
		is_connected = Initialize();
		if (is_connected)
		{
			start_accept();
		}
		else
		{
			AddLog("无有效IP");
		}

	}
	~CNode(void){}

private:
	bool Initialize();
	void AddNodes();
	void ParseProj();
	void Distribute();
	void Work();

private:
	void start_accept();
	void start_scan();
	void start_ping();
	void handle_accept(session* new_session, const boost::system::error_code& error);
	void handle_accept_file(session* new_session, file_struct* file, const boost::system::error_code& error);
	void handle_connect(session* new_session, const boost::system::error_code& error);
	void handle_connect_msg(session* new_session, msg_struct* msg, const boost::system::error_code& error);
	void handle_msg(string ip, const char* msg);
	void handle_result(MsgType mt, string ip, bool is_sender = false);
	void send_metafile(session* new_session, addr_struct* addr, const boost::system::error_code& error);

public:
	bool IsConnected();
	bool IsScanFinished();
	bool IsMaster();
	bool InCharge();
	NodeType GetNodeType();
	int GetNodeNum();
	string& GetIP();
	void Start();
	void AddLog(string log);
	std::vector<string>& GetIPList();
	std::deque<string>& GetLogList();
	
private:
	NodeType nt_;
	string ip_;
	string next_ip;
	string master_ip;
	unsigned short listen_port;
	bool is_connected;
	bool is_scan_finished;
	bool is_ping_busy;
	bool is_busy;
	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;
	tcp::acceptor file_acceptor_;
	std::vector<string> ip_list;
	std::vector<node_struct> available_list;
	std::deque<string> log_list;
	std::vector<task_struct> task_list;
	fstream outfile;
	unsigned int limit_filenum_to_transfer;
	unsigned int cur_filenum;
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

	void recv_file(string filename, unsigned __int64 filesize)
	{
		filename = filename.substr(1, filename.size()-1);
		file.open(filename, ios::out|ios::binary);
		if (filesize > max_length)
		{
			boost::asio::async_read(socket_,
				boost::asio::buffer(data_in, max_length),
				boost::asio::transfer_exactly(max_length),
				boost::bind(&session::read_file, this,
				filesize-max_length,
				boost::asio::placeholders::error));
		}
		else
		{
			boost::asio::async_read(socket_,
				boost::asio::buffer(data_in, static_cast<size_t>(filesize)),
				boost::asio::transfer_exactly(static_cast<size_t>(filesize)),
				boost::bind(&session::read_over, this,
				filesize,
				boost::asio::placeholders::error));
		}
	}

	void send_file(string filename, unsigned __int64 filesize)
	{
		file.open(filename, ios::in|ios::binary);
		if (filesize > max_length)
		{
			file.read(data_out, max_length);
			boost::asio::async_write(socket_,
				boost::asio::buffer(data_out, max_length),
				boost::asio::transfer_exactly(max_length),
				boost::bind(&session::write_file, this,
				filesize-max_length,
				boost::asio::placeholders::error));
		}
		else
		{
			file.read(data_out, filesize);
			boost::asio::async_write(socket_,
				boost::asio::buffer(data_out, static_cast<size_t>(filesize)),
				boost::asio::transfer_exactly(static_cast<size_t>(filesize)),
				boost::bind(&session::write_over, this,
				filesize,
				boost::asio::placeholders::error));
		}
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
			unsigned __int64 bytes_to_transfer = _strtoui64(data_in, &stopstring, 16);
			read_content(bytes_to_transfer-3, error);
		}
		else
		{
			delete this;
		}
	}

	void read_content(unsigned __int64 bytes_to_transfer, const boost::system::error_code& error)
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
					boost::asio::buffer(data_in, static_cast<size_t>(bytes_to_transfer)),
					boost::asio::transfer_exactly(static_cast<size_t>(bytes_to_transfer)),
					boost::bind(&session::handle_read, this,
					boost::asio::placeholders::error));
			}
		}
		else
		{
			delete this;
		}
	}

	void read_file(unsigned __int64 bytes_to_transfer, const boost::system::error_code& error)
	{
		if (!error)
		{
			file.write(data_in, max_length);
			if (bytes_to_transfer > max_length)
			{
				boost::asio::async_read(socket_,
					boost::asio::buffer(data_in, max_length),
					boost::asio::transfer_exactly(max_length),
					boost::bind(&session::read_file, this,
					bytes_to_transfer-max_length,
					boost::asio::placeholders::error));
			}
			else
			{
				boost::asio::async_read(socket_,
					boost::asio::buffer(data_in, static_cast<size_t>(bytes_to_transfer)),
					boost::asio::transfer_exactly(static_cast<size_t>(bytes_to_transfer)),
					boost::bind(&session::read_over, this,
					bytes_to_transfer,
					boost::asio::placeholders::error));
			}
		}
		else
		{
			owner_->AddLog(error.message());
			boost::system::error_code ec;
			boost::asio::ip::tcp::endpoint endpoint = socket_.remote_endpoint(ec);
			if (!ec)
			{
				owner_->handle_result(MT_METAFILE_FAIL, endpoint.address().to_string());
			}
			delete this;
		}
	}

	void write_file(unsigned __int64 bytes_to_transfer, const boost::system::error_code& error)
	{
		if (!error)
		{
			if (bytes_to_transfer > max_length)
			{
				file.read(data_out, max_length);
				boost::asio::async_write(socket_,
					boost::asio::buffer(data_out, max_length),
					boost::asio::transfer_exactly(max_length),
					boost::bind(&session::write_file, this,
					bytes_to_transfer-max_length,
					boost::asio::placeholders::error));
			}
			else
			{
				file.read(data_out, bytes_to_transfer);
				boost::asio::async_write(socket_,
					boost::asio::buffer(data_out, static_cast<size_t>(bytes_to_transfer)),
					boost::asio::transfer_exactly(static_cast<size_t>(bytes_to_transfer)),
					boost::bind(&session::write_over, this,
					bytes_to_transfer,
					boost::asio::placeholders::error));
			}
		}
		else
		{
			owner_->AddLog(error.message());
			file.close();
			boost::system::error_code ec;
			boost::asio::ip::tcp::endpoint endpoint = socket_.remote_endpoint(ec);
			if (!ec)
			{
				owner_->handle_result(MT_METAFILE_FAIL, endpoint.address().to_string(), true);
			}
			delete this;
		}
	}

	void read_over(unsigned __int64 last_length, const boost::system::error_code& error)
	{
		boost::system::error_code ec;
		boost::asio::ip::tcp::endpoint endpoint = socket_.remote_endpoint(ec);
		if (!error)
		{
			owner_->AddLog("接收完毕");
			if (!ec)
			{
				owner_->handle_result(MT_METAFILE_FINISH, endpoint.address().to_string());
			}
		}
		else
		{
			owner_->AddLog(error.message());
			if (!ec)
			{
				owner_->handle_result(MT_METAFILE_FAIL, endpoint.address().to_string());
			}
		}
		file.write(data_in, last_length);
		file.close();
		delete this;
	}

	void write_over(unsigned __int64 last_length, const boost::system::error_code& error)
	{
		if (!error)
		{
			owner_->AddLog("发送完毕");
		}
		else
		{
			owner_->AddLog(error.message());
			boost::system::error_code ec;
			boost::asio::ip::tcp::endpoint endpoint = socket_.remote_endpoint(ec);
			if (!ec)
			{
				owner_->handle_result(MT_METAFILE_FAIL, endpoint.address().to_string(), true);
			}
		}
		file.close();
		delete this;
	}

	void handle_read(const boost::system::error_code& error)
	{
		if (!error)
		{
			boost::system::error_code ec;
			boost::asio::ip::tcp::endpoint endpoint = socket_.remote_endpoint(ec);
			if (ec)
			{
				delete this;
			}
			else
			{
				owner_->handle_msg(endpoint.address().to_string(), data_in);
			}
		}
		delete this;
	}

private:
	tcp::socket socket_;
	enum {max_length = 2048};
	char data_out[2048];
	char data_in[2048];
	CNode* owner_;
	fstream file;
};

