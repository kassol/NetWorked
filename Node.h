#pragma once
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread.hpp>
#include <boost/smart_ptr.hpp>

using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;  


class session
{
public:
	session(boost::asio::io_service& io_service)
		: socket_(io_service)
	{
	}

	tcp::socket& socket()
	{
		return socket_;
	}

	void start()
	{
		socket_.async_read_some(boost::asio::buffer(data_, max_length),
			boost::bind(&session::handle_read, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}

	void close()
	{
		socket_.close();
	}

private:
	void handle_read(const boost::system::error_code& error,
		size_t bytes_transferred)
	{
		if (!error)
		{
			boost::asio::async_write(socket_,
				boost::asio::buffer(data_, bytes_transferred),
				boost::bind(&session::handle_write, this,
				boost::asio::placeholders::error));
		}
		else
		{
			delete this;
		}
	}

	void handle_write(const boost::system::error_code& error)
	{
		if (!error)
		{
			socket_.async_read_some(boost::asio::buffer(data_, max_length),
				boost::bind(&session::handle_read, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}
		else
		{
			delete this;
		}
	}

private:
	tcp::socket socket_;
	enum { max_length = 1024 };
	char data_[max_length];
};

class CNode : public boost::enable_shared_from_this<CNode>
	, boost::noncopyable
{
public:
	typedef boost::shared_ptr<CNode>nodeptr;
	enum NodeType{NT_MASTER, NT_NORMAL};
public:
	CNode(boost::asio::io_service& io_service, short port)
		: nt_(NT_NORMAL)
		, ip_("")
		, next_ip("")
		, master_ip("")
		, io_service_(io_service)
		, timer_(io_service)
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
	void close_session(session* new_session);

public:
	bool IsConnected();
	bool IsMaster();
	bool InCharge();
	NodeType GetNodeType();
	string& GetIP();
	void ScanNode();
	void Start();
	std::vector<string>& GetIPList();
	
private:
	NodeType nt_;
	string ip_;
	string next_ip;
	string master_ip;
	int listen_port;
	bool is_connected;
	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;
	deadline_timer timer_;
	std::vector<string> ip_list;
};

