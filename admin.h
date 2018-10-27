#ifndef ADMIN_H
#define ADMIN_H

#include <boost/thread/detail/singleton.hpp>
#include <stdexcept>
#include <string>

#include "http_server.h"
#include "stat_base.h"
#include "response.h"
#include "util.h"
#include "log.h"

using namespace boost::detail::thread;

class path_not_supported : public std::exception
{
	const std::string m_what;

public:
	explicit path_not_supported(const std::string& arg_what);
	const char* what() const noexcept;
};

template <typename STAT_TYPE>
class admin : public net::http_server
{
public:
	admin(net::boost_socket_ptr arg_socket) : net::http_server(arg_socket) {}

	virtual void handle_process(const request& req)
	{
		response resp;

		if (*req.path() == "/stat.json") {

			auto& prof = singleton<profile<STAT_TYPE>>::instance();

			resp.data().lock()->assign(prof.get());
			resp.add_header("Content-type", "application/json; charset=utf-8");
			resp.set_status(S200);

		} else if (*req.path() == "/log.json") {

			std::string level = req.get<std::string>("l", "");
			lo::set_log_level(lo::trans_level(level));

			resp.data().lock()->assign("{\n\t\"current_level\":\"" + lo::get_level() + "\"\n}\n");
			resp.add_header("Content-type", "application/json; charset=utf-8");
			resp.set_status(S200);

		} else throw path_not_supported(*req.path());

		resp.set_ver(req.ver());
		http_server::send(*resp.to_raw().lock());
	}
};

#endif
