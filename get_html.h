#ifndef GET_HTML_H
#define GET_HTML_H

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio.hpp>
#include <string>
#include <memory>

#include "response.h"
#include "request.h"
#include "types.h"

using boost::asio::ip::tcp;

class get_html_c
{
private:
	boost::asio::io_service		m_io_service;
	tcp::socket					m_socket;
	boost::asio::deadline_timer	m_deadline;

	sptr_cstr					m_data_send;
	sptr_str					m_data_recv;

	size_t						m_size_send;
	size_t						m_size_recv;

	std::shared_ptr<response>	m_res;

public:
	get_html_c();

private:
	void connect(const std::string& ip, int port);
	void handler_connect(const boost::system::error_code& error);
	void handler_send(const boost::system::error_code& error, size_t size);
	void handler_recv(const boost::system::error_code& error, size_t size);
	void handler_wait(const boost::system::error_code& error);

public:
	std::weak_ptr<response> operator()(const std::string& addr, request& req, size_t timeout);
};

std::weak_ptr<response> get_html(const std::string upstream, request& req, size_t timeout = 30000);

#endif
