#include "http_server.h"
#include "response.h"
#include "log.h"

namespace net
{

http_server::http_server(boost_socket_ptr arg_socket) : socket(arg_socket) {}

void http_server::handle_accept(const boost::system::error_code &err)
{
	socket::receive();
}

void http_server::handle_receive(const boost::system::error_code &err, size_t bytes)
{
	if (get_data_size() > 1) {
		lo::l(lo::IN) << "Received: " << get_data_size() << " bytes";

		request req;
		req.set_raw(get_data());

		bool parse_ok = false;

		try {
			parse_ok = req.parse();
		} catch (...) {
			lo::l(lo::ERROR) << "Can't parse the request: " << *req.raw();
			response res;
			res.set_status(S500);
			send(*res.to_raw().lock());
			return;
		}

		if (parse_ok) {
			// Запрос удачно распарсен. Можно обрабатывать.
			lo::l(lo::IN) << "Request: " << *req.path();
			handle_process(req);
		} else {
			socket::receive();
		}
	}
}

void http_server::handle_send(const boost::system::error_code &err)
{
	socket::close();
}

void http_server::send(const std::string& line)
{
	socket::send(line);
	lo::l(lo::OUT) << "Sent: " << line;
}

} // namespace net 
