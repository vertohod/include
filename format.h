#ifndef FORMAT_H
#define FORMAT_H

#include <string> 
#include "types.h"

template <typename T>
T format(const std::string& arg) { return arg; }

template <>
char format<char>(const std::string& arg);

template <>
unsigned char format<unsigned char>(const std::string& arg);

template <>
short format<short>(const std::string& arg);

template <>
unsigned short format<unsigned short>(const std::string& arg);

template <>
int format<int>(const std::string& arg);

template <>
unsigned int format<unsigned int>(const std::string& arg);

template <>
long format<long>(const std::string& arg);

template <>
unsigned long format<unsigned long>(const std::string& arg);

template <>
double format<double>(const std::string& arg);

template <>
bool format<bool>(const std::string& arg);

#ifndef __linux__
template <>
uint64_t format<uint64_t>(const std::string& arg);
#endif

template <>
std::shared_ptr<std::vector<sptr_str>> format<std::shared_ptr<std::vector<sptr_str>>>(const std::string& arg);

template <>
std::shared_ptr<std::vector<OID>> format<std::shared_ptr<std::vector<OID>>>(const std::string& arg);


// Для умных указателей
template <typename T>
T format(const sptr_cstr& arg) { return format<T>(*arg); }

// Специальная функция

template <typename T>
void format(const std::string& arg, T& res);


// Обратное преобразование

namespace std
{
	inline const std::string& to_string(const std::string& arg) { return arg; }

	template <typename T>
	const std::string& to_string(const sptr_string<T>& arg) { return *arg; }
}

template <typename T>
const std::string format(T arg) { return std::to_string(arg); }

template <>
const std::string format<char>(char arg);

template <>
const std::string format<unsigned char>(unsigned char arg);

template <>
const std::string format<bool>(bool arg);

template <>
const std::string format<std::shared_ptr<std::vector<sptr_str>>>(std::shared_ptr<std::vector<sptr_str>>);

template <>
const std::string format<std::shared_ptr<std::vector<OID>>>(std::shared_ptr<std::vector<OID>>);

#endif
