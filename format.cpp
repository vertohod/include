#include <stdlib.h>

#include "stream.h"
#include "format.h"
#include "util.h"

template <>
char format<char>(const std::string& arg) { return util::atoi<char>(arg); }

template <>
unsigned char format<unsigned char>(const std::string& arg) { return static_cast<unsigned char>(util::atoi<unsigned char>(arg)); }

template <>
short format<short>(const std::string& arg) { return util::atoi<short>(arg); }

template <>
unsigned short format<unsigned short>(const std::string& arg) { return util::atoi<unsigned short>(arg); }

template <>
int format<int>(const std::string& arg) { return util::atoi<int>(arg); }

template <>
unsigned int format<unsigned int>(const std::string& arg) { return util::atoi<unsigned int>(arg); }

template <>
long format<long>(const std::string& arg) { return util::atoi<long>(arg); }

template <>
unsigned long format<unsigned long>(const std::string& arg) { return util::atoi<unsigned long>(arg); }

template <>
double format<double>(const std::string& arg) { return atof(arg.c_str()); }

template <>
bool format<bool>(const std::string& arg) { return arg == "true"; }

#ifndef __linux__
template <>
uint64_t format<uint64_t>(const std::string& arg) { return util::atoi<uint64_t>(arg); }
#endif

template <>
std::shared_ptr<std::vector<sptr_str>> format<std::shared_ptr<std::vector<sptr_str>>>(const std::string& arg)
{
	auto res = std::make_shared<std::vector<sptr_str>>();
	auto vals = util::split(arg, ",");
	for (auto& val : *vals) res->push_back(sptr_str(val));
	return res;
}

template <>
std::shared_ptr<std::vector<OID>> format<std::shared_ptr<std::vector<OID>>>(const std::string& arg)
{
	auto res = std::make_shared<std::vector<OID>>();
	auto vals = util::split(arg, ",");
	for (auto& val : *vals) res->push_back(format<OID>(val));
	return res;
}

// Специальная функция

template <typename T>
void format(const std::string& arg, T& res) { res = format<T>(arg); }


// Обратное преобразование

template <>
const std::string format<char>(char arg) { char ch[2] = {0}; ch[0] = arg; return std::string(ch); }

template <>
const std::string format<unsigned char>(unsigned char arg) { return util::itoa(arg); }

template <>
const std::string format<bool>(bool arg) { return arg ? "true" : "false"; }

template <>
const std::string format<std::shared_ptr<std::vector<sptr_str>>>(std::shared_ptr<std::vector<sptr_str>> arg)
{
	if (!arg) return std::string();

	sptr_str res;
	sp::stream st(*res);

	bool flag = false;
	for (auto& val : *arg) {
		if (flag) st << ","; else flag = true;
		st << val;
	}
	return *res;
}

template <>
const std::string format<std::shared_ptr<std::vector<OID>>>(std::shared_ptr<std::vector<OID>> arg)
{
	if (!arg) return std::string();

	sptr_str res;
	sp::stream st(*res);

	bool flag = false;
	for (auto& val : *arg) {
		if (flag) st << ","; else flag = true;
		st << val;
	}
	return *res;
}
