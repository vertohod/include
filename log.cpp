#include "log.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>

#include <stdio.h>
#include <fcntl.h>
#include "util.h"

lo::log_level_t lo::log_level = lo::INFO;
std::string lo::m_path;

void lo::set_output(const std::string& arg_path)
{
    const int STDOUT = 1;
    const int STDERR = 2;

    if (arg_path.empty()) return;

	m_path = arg_path;

	int fd_log = open(arg_path.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
	if (-1 == fd_log) throw std::runtime_error("failed to open log file " + arg_path);

	int res_dup = dup2(fd_log, STDOUT);
	if (-1 == res_dup) throw std::runtime_error("failed to redirect STDOUT");

	res_dup = dup2(fd_log, STDERR);
	if (-1 == res_dup) throw std::runtime_error("failed to redirect STDERR");
	close(fd_log);

	std::cout.exceptions(std::ostream::failbit | std::ostream::badbit);
}

lo::l::l(log_level_t ar_type) : m_type(ar_type)
{
	if (m_type >= log_level) {
		try {
			std::cout << get_time() << " [" << boost::this_thread::get_id() << "]: " << get_level_str(m_type) << ": ";
		} catch (const std::ostream::failure& e) {
			set_output(m_path);
			std::cout << get_time() << " [" << boost::this_thread::get_id() << "]: " << get_level_str(m_type) << ": ";
		}
	}
}

lo::l::l(const l& obj)
{
	m_type = obj.m_type;
}

lo::l::~l()
{
	if (m_type >= log_level) std::cout << std::endl;
}

void lo::set_log_level(log_level_t arg_level)
{
	log_level = arg_level;
}
	
std::string lo::get_level_str(log_level_t arg_level)
{
	std::string out;

	switch (arg_level) {
		case TRASH:
			out = "TRASH  ";
			break;
		case DB:
			out = "DB     ";
			break;
		case DEBUG:
			out = "DEBUG  ";
			break;
		case INFO:
			out = "INFO   ";
			break;
		case IN:
			out = "IN     ";
			break;
		case OUT:
			out = "OUT    ";
			break;
		case WARNING:
			out = "WARNING";
			break;
		case ERROR:
			out = "ERROR  ";
			break;
		case FATAL:
			out = "FATAL  ";
			break;
	}

	return out;
}

lo::log_level_t lo::trans_level(const std::string& str)
{
	if (str == "TRASH") return TRASH;
	else if (str == "DB") return DB;
	else if (str == "DEBUG") return DEBUG;
	else if (str == "INFO") return INFO;
	else if (str == "IN") return IN;
	else if (str == "OUT") return OUT;
	else if (str == "WARNING") return WARNING;
	else if (str == "ERROR") return ERROR;
	else if (str == "FATAL") return FATAL;
	else return FATAL;
}

std::string lo::get_level()
{
	return util::trim(get_level_str(log_level));
}

std::string lo::get_time()
{
	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	return boost::posix_time::to_simple_string(now);
}
