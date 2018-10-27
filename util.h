#ifndef UTIL_H
#define UTIL_H

#include <boost/algorithm/string.hpp>
#include <boost/thread/thread.hpp>
#include <sys/types.h>
#include <algorithm>
#include <stdexcept>
#include <unistd.h>
#include <string>
#include <vector>

#include "format.h"
#include "types.h"

namespace util
{

void daemonize();
void savepid(const std::string& path);

template<typename T>
std::string itoa(T arg, int base = 10)
{
	if (0 == arg) return std::string("0");
	char alph[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";	
	if (sizeof(alph) < base) throw std::runtime_error("(util::itoa) Выход за допустимый диапазон");
	std::string result;
	while (0 != arg) {
		result += alph[arg % base];
		arg /= base;
	}
	std::reverse(result.begin(), result.end());
	return result;
}

template<typename T>
T set_all_bits()
{
	T res = 0;
	for (T temp = 1; (temp != res) && (temp > 0); temp = (temp << 1) | 1) {
		res = temp;
	}
	return res;
}

template<typename T>
T atoi(std::string str, int base = 10)
{
	const std::string alph("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");	
	T test = set_all_bits<T>();
	test = set_all_bits<T>();
	std::string test_str = itoa(test, base);
	if (str.size() <=  test_str.size()) {
		size_tt diff = test_str.size() - str.size();
		for (size_tt i = 0; i < diff; ++i) str = '0' + str;	
	} else throw std::runtime_error("(util::atoi) Длина строки превышает допустимый диапазон указанного типа");
	if (str > test_str) throw std::runtime_error("(util::atoi) Число в строке превышает диапазон указанного типа"); 
	T result = 0;
	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
		size_tt pos = alph.find(*it);
		if (pos >= base) throw std::runtime_error("(util::atoi) В строке использован недопустимый символ");
		result = result * base + pos;
	}
	return result;
}

std::unique_ptr<std::vector<std::string>> split(const std::string& data, const std::string& del);

std::string trim(const std::string& line, const char* t = " \t\n\r\f\v");
uptr_cstr escape_html(const std::string& str);
size_tt length_utf8(const std::string& arg_str);
size_tt offset_utf8(const std::string& arg_str, size_tt symbols);
std::string ins(std::string, const std::string&);
pid_t exec_command(const std::string&);
void rename(const std::string&, const std::string&);

struct passwd
{
	std::string		pw_name;       /* username */
	std::string		pw_passwd;     /* user password */
	uid_t			pw_uid;        /* user ID */
	gid_t			pw_gid;        /* group ID */
	std::string		pw_gecos;      /* user information */
	std::string		pw_dir;        /* home directory */
	std::string		pw_shell;      /* shell program */
};

struct group
{
	std::string					gr_name;
	std::string					gr_passwd;
	gid_t						gr_gid;
	std::vector<std::string>	gr_mem;
};

passwd getpwnam(const std::string&);
group getgrnam(const std::string&);
time_tt get_gtime();
std::string get_local_time();
sptr_str get_random_string(const std::string& alphabet, size_tt length);
size_tt random();
std::string remove_end_slash(const std::string& path);

inline void sleep(size_tt arMillisecond)
{
	boost::this_thread::sleep_for(boost::chrono::milliseconds(arMillisecond));
}

template<typename RES, typename MP>
RES get_val(const MP& arg_map, const std::string& arg_key, RES arg_value)
{
	auto it = arg_map.find(arg_key);
	if (it == arg_map.end()) {
		return arg_value;
	}
	return format<RES>(it->second);
}

template<typename RES, typename MP>
RES get_val(const MP& arg_map, const std::string& arg_key)
{
	auto it = arg_map.find(arg_key);
	if (it == arg_map.end()) throw std::runtime_error("Field " + arg_key + " is not found");
	return format<RES>(it->second);
}

template<typename T>
std::unique_ptr<const std::vector<T>> string_to_vector(const std::string& str, const std::string& del)
{
	std::unique_ptr<std::vector<T>> res;

	if (!str.empty()) {
		auto vect = util::split(str, del);
		for (auto& val : *vect) res->push_back(format<T>(val));
	}

	return res;
}

template <typename T>
void hash_combine(size_tt& seed, const T& obj)
{
	seed ^= std::hash<T>()(obj) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}


} // namespace util

#endif

