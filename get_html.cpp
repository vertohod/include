#include <boost/bind.hpp>
#include "get_html.h"

get_html_c::get_html_c() :
	m_io_service(), m_socket(m_io_service), m_deadline(m_io_service),
	m_size_send(0), m_size_recv(0)
{
	m_data_recv = m_res->raw();
}

void get_html_c::connect(const std::string& ip, int port)
{
	tcp::endpoint endpoint(boost::asio::ip::address_v4::from_string(ip), port);

	m_socket.async_connect(endpoint, boost::bind(&get_html_c::handler_connect, this, _1));
}

void get_html_c::handler_connect(const boost::system::error_code& error)
{
	if (error) return; 

	m_socket.async_send(boost::asio::buffer(reinterpret_cast<const void*>(m_data_send->c_str()), m_data_send->size()), 0, boost::bind(&get_html_c::handler_send, this, _1, _2));
}

void get_html_c::handler_send(const boost::system::error_code& error, size_t size)
{
	if (error) return; 

	m_size_send += size;

	if (m_size_send < m_data_send->size()) {
		m_socket.async_send(boost::asio::buffer(reinterpret_cast<const void*>(&(*m_data_send)[m_size_send]), m_data_send->size() - m_size_send), 0, boost::bind(&get_html_c::handler_send, this, _1, _2));
	} else {
		m_data_recv->resize(512000); // 500 Кб
		m_socket.async_receive(boost::asio::buffer(reinterpret_cast<void*>(&(*m_data_recv)[0]), m_data_recv->size()), 0, boost::bind(&get_html_c::handler_recv, this, _1, _2));
	}
}

void get_html_c::handler_recv(const boost::system::error_code& error, size_t size)
{
	if (error) return; 

	m_size_recv += size;
	m_data_recv->resize(m_size_recv);

	if (m_res->parse()) {
		m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		m_socket.close();

		// Почему-то перестал прерываться run
		throw 1;

	} else {
		if (m_data_recv->size() == m_size_recv) m_data_recv->resize(m_data_recv->size() * 1.5);
		m_socket.async_receive(boost::asio::buffer(reinterpret_cast<void*>(&(*m_data_recv)[m_size_recv]), m_data_recv->size() - m_size_recv), 0, boost::bind(&get_html_c::handler_recv, this, _1, _2));
	}
}

void get_html_c::handler_wait(const boost::system::error_code& error)
{
	if (error) return; 

	m_socket.close();
}

std::weak_ptr<response> get_html_c::operator()(const std::string& addr, request& req, size_t timeout)
{
	m_data_send = req.to_raw();

	std::string ip;
	int port;

	auto pos = addr.find(":");
	if (pos != std::string::npos) {
		ip = addr.substr(0, pos);
		port = atoi(addr.substr(pos + 1, addr.size() - pos - 1).c_str());
	} else {
		ip = addr;
		port = 80;
	}

	m_deadline.expires_from_now(static_cast<boost::posix_time::time_duration>(boost::posix_time::milliseconds(timeout)));
	m_deadline.async_wait(boost::bind(&get_html_c::handler_wait, this, _1));

	connect(ip, port);

	try {
		m_io_service.run();
	} catch (...) {}

	return m_res;
}

std::weak_ptr<response> get_html(const std::string upstream, request& req, size_t timeout)
{
	get_html_c gh;
	
	return gh(upstream, req, timeout);
}
