#include <sys/stat.h>
#include <sys/wait.h>
#include <syslog.h>
#include <signal.h>

#include <exception>
#include <stdexcept>
#include <clocale>
#include <cstdlib>
#include <fstream>
#include <random>
#include <ctime>

#include "util.h"

namespace util
{

void daemonize()
{
	umask(0);

	pid_t pid;
	if ((pid = fork()) < 0) exit(1);
	else if (pid != 0) exit(0);

	setsid();

	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);	
	sa.sa_flags = 0;

	if (sigaction(SIGHUP, &sa, NULL) < 0) exit(1);

	if ((pid = fork()) < 0) exit(1);
	else if (pid != 0) exit(0);

	if (chdir("/") < 0) exit(1);
}

void savepid(const std::string& path)
{
	FILE* f;
	f = fopen(path.c_str(), "w+");
	if (f) {
		fprintf(f, "%u", getpid());
		fclose(f);
	}
}

std::unique_ptr<std::vector<std::string>> split(const std::string& data, const std::string& del)
{
	auto res = std::make_unique<std::vector<std::string>>();
	boost::split(*res, data, boost::is_any_of(del), boost::token_compress_on);
	return res;
}

std::string trim(const std::string& line, const char* t)
{
	std::string temp(line);
	temp.erase(0, temp.find_first_not_of(t));
	temp.erase(temp.find_last_not_of(t) + 1);
	return temp;
}

uptr_cstr escape_html(const std::string& str)
{
	uptr_str res;

	for (auto ch : str) {
		if (ch == '&')		res->append("&amp;");
		else if (ch == '<')	res->append("&lt;");
		else if (ch == '"')	res->append("&quot;");
		else if (ch == '>')	res->append("&gt;");
		else if (static_cast<unsigned char>(ch) <= 0x20 || ch == 0x7f) res->append(" ");
		else *res += ch;
	}
	return res;
}

size_tt length_utf8(const std::string& arg_str)
{
	setlocale(LC_ALL, "ru_RU.UTF-8");
	size_tt size = arg_str.size();
    size_tt length = 0;
	for (size_tt offset = 0; offset < size; ++length) {
		offset += mblen(&arg_str[offset], size - offset);
	}
	return length;
}

size_tt offset_utf8(const std::string& arg_str, size_tt symbols)
{
	setlocale(LC_ALL, "ru_RU.UTF-8");
	size_tt size = arg_str.size();
    size_tt offset = 0;
	for (size_tt length = 0; offset < size && length < symbols; ++length) {
		offset += mblen(&arg_str[offset], size - offset);
	}
	return offset;
}

std::string ins(std::string line, const std::string& arg)
{
	std::string del("\%s");

	auto pos = line.find(del);
	if (pos != std::string::npos) {
		line.replace(pos, del.length(), arg);
	}
	return line;
}

pid_t exec_command(const std::string& command)
{
	static const std::string path("/bin/sh");
	static const std::string arg1("-c");

	const char *argv[4];

	argv[0] = path.c_str();
	argv[1] = arg1.c_str();
	argv[2] = command.c_str();
	argv[3] = NULL;

	pid_t pid = fork();
	if (pid == 0) {
		execv(path.c_str(), const_cast<char *const *>(argv));
		exit(1);
	}
	return pid;
}

void rename(const std::string& name_old, const std::string& name_new)
{
	if (0 > ::rename(name_old.c_str(), name_new.c_str())) {
		throw std::runtime_error("Failed to rename file: " + name_old);
	}
}

passwd getpwnam(const std::string& arg_user)
{
    std::ifstream   ifs;
    std::string     str_in;

    ifs.open("/etc/passwd", std::ifstream::in);
    if (ifs.fail()) throw std::runtime_error("failed to open file /etc/passwd");
    for (char ch = ifs.get(); ifs.good(); ch = ifs.get()) {
        str_in.push_back(ch);
    }
    ifs.close();

	passwd pwd;

	auto lines = util::split(str_in, "\n");
	for (auto& line : *lines) {
		auto fields = util::split(line, ":");
		if (fields->size() == 7) {
			if (fields->at(0) == arg_user) {
				pwd.pw_name = fields->at(0);
				pwd.pw_passwd = fields->at(1);
				pwd.pw_uid = format<uid_t>(fields->at(2));
				pwd.pw_gid = format<gid_t>(fields->at(3));
				pwd.pw_gecos = fields->at(4);
				pwd.pw_dir = fields->at(5);
				pwd.pw_shell = fields->at(6);
				return pwd;
			}
		}
	}
	return pwd;
}

group getgrnam(const std::string& arg_group)
{
    std::ifstream   ifs;
    std::string     str_in;

    ifs.open("/etc/group", std::ifstream::in);
    if (ifs.fail()) throw std::runtime_error("failed to open file /etc/group");
    for (char ch = ifs.get(); ifs.good(); ch = ifs.get()) {
        str_in.push_back(ch);
    }
    ifs.close();

	group grp;

	auto lines = util::split(str_in, "\n");
	for (auto& line : *lines) {
		auto fields = util::split(line, ":");
		if (fields->size() == 4) {
			if (fields->at(0) == arg_group) {
				grp.gr_name = fields->at(0);
				grp.gr_passwd = fields->at(1);
				grp.gr_gid = format<gid_t>(fields->at(2));
				grp.gr_mem = *util::split(fields->at(3), ",");
			}
		}
	}

	return grp;
}

// Время по гринвичу
time_tt get_gtime()
{
    time_t rawtime;

    time(&rawtime);

    return rawtime;
}

std::string get_local_time()
{
    time_t rawtime;
    struct tm *tm_temp;
    time(&rawtime);
    tm_temp = localtime(&rawtime);

    char buffer[160];
      
    strftime(buffer, 160, "%a, %e-%b-%G %T", tm_temp);

	return std::string(buffer);
}

sptr_str get_random_string(const std::string& alphabet, size_tt length)
{
	static std::default_random_engine gn;

	if (alphabet.size() == 0) return uptr_str();

	std::uniform_int_distribution<size_tt> distribution(0, alphabet.size() - 1);
	sptr_str res;

	while (res->size() < length) {
		*res += alphabet[distribution(gn)];
	}

	return res;
}

size_tt random()
{
    static std::mt19937_64 generator;
    static size_tt value = 0;
    static std::uniform_int_distribution<size_tt> distribution(time(NULL), --value);

    return distribution(generator);
}

std::string remove_end_slash(const std::string& path)
{
	if (path.empty()) return path;

	if (path[path.size() - 1] == '/') return path.substr(0, path.size() - 1);

	return path;
}

} // namespace util

