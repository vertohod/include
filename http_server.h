#ifndef HTTP_H
#define HTTP_H

#include "request.h"
#include "socket.h"

namespace net
{

class http_server : public socket
{
public:
	http_server(boost_socket_ptr);

	virtual void handle_accept(const boost::system::error_code &err);
	virtual void handle_receive(const boost::system::error_code &err, size_t bytes);
	virtual void handle_send(const boost::system::error_code &err);
	virtual void handle_process(const request&) = 0;
	void send(const std::string&);
};

}

#endif
