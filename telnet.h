#ifndef TELNET_H
#define TELNET_h

#include <string>
#include "socket.h"

namespace telnet
{

class socket_tl : public http::socket
{
	std::string m_request;

public:
	socket_tl(http::boost_socket_ptr arg_socket);

	virtual void handle_accept(const boost::system::error_code &err);
	virtual void handle_send(const boost::system::error_code &err);
	virtual void handle_receive(const boost::system::error_code &err, size_t bytes);
	void send(const std::string& arg_line);
	void processor();
};

} // namespace telnet
#endif
