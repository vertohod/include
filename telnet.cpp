#include <iostream>
#include "util.h"
#include "telnet.h"

namespace telnet
{

std::string str_ok("Hello\n");
std::string str_goodbye("Goodbye...\n");

socket_tl::socket_tl(http::boost_socket_ptr arg_socket) : socket(arg_socket)
{
}

socket_tl::socket_tl(http::boost_socket_ptr arg_socket) : socket(arg_socket)
{
}

void socket_tl::handle_accept(const boost::system::error_code &err)
{
	const cvector data(str_ok.begin(), str_ok.end());
	socket::send(data);
}

void socket_tl::handle_send(const boost::system::error_code &err)
{
	if (get_status() == ST_SEND_CLOSE) socket::close();
	else socket::receive();
}

void socket_tl::handle_receive(const boost::system::error_code &err, size_t bytes)
{
	if (get_data_size() > 1) {
		cvector data;
		get_data(data);	
		m_request.append(data.begin(), data.end());
		processor();
	}
}
	
void socket_tl::send(const std::string& arg_line)
{
	const cvector data(arg_line.begin(), arg_line.end());
	socket::send(data);
}

void socket_tl::processor()
{
	auto commands = util::split(m_request, "\r\n");

	if (commands.size() > 1) {
		m_request.clear();

		for (auto const& command : commands) {
			if (command.empty()) continue;
			std::cout << "Command: " << command << std::endl;
			if (command == "quit" || command == "QUIT" || command == "Quit") {
				set_status(ST_SEND_CLOSE);
				send(str_goodbye);
				return;
			}
		}
	}
	socket::receive();
}

} // namespace telnet
