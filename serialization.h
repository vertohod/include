#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include "format.h"
#include "stream.h"
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <unordered_map>

namespace serialization
{

std::string escape(const std::string& arg_value);

void begin(std::string& buf);
void end(std::string& buf);
void comma(std::string& buf);
void enter(std::string& buf);
void tab(std::string& buf, int level = 1);

template <typename T>
void s(std::string& buf, const T& arg_obj, int level)
{
	buf.append(format(arg_obj));
}

template <>
void s<std::string>(std::string& buf, const std::string& arg_obj, int level);

template <>
void s<char>(std::string& buf, const char& arg_obj, int level);

//////////////////////////////////////////////////////////////////////////

template <typename T>
void s(std::string& buf, const std::string& arg_name, const T& arg_obj, int level)
{
	tab(buf, level); buf.append("\"" + arg_name + "\":");
	s(buf, arg_obj, level);
}

template <typename T>
void s(std::string& buf, const std::string& arg_name, const std::vector<T>& arg_obj, int level)
{
	tab(buf, level); buf.append("\"" + arg_name + "\":[");
	bool first = true;
	for (auto el : arg_obj) {
		if (!first) buf.append(","); else first = false;

		s(buf, el, 0);
	}
	tab(buf, level); buf.append("]");
}

template <typename K, typename V>
void s(std::string& buf, const std::string& arg_name, const std::map<K,V>& arg_obj, int level)
{
	tab(buf, level); buf.append("\"" + arg_name + "\":{");
	bool first = true;
	for (auto pr : arg_obj) {
		if (!first) buf.append(","); else first = false;

		// format для того, чтоб ключи всегда были строковыми и брались в кавычки
		s(buf, format(pr.first), 0); buf.append(":"); s(buf, pr.second, 0);
	}
	tab(buf, level); buf.append("}");
}

template <typename K, typename V>
void s(std::string& buf, const std::string& arg_name, const std::unordered_map<K,V>& arg_obj, int level)
{
	tab(buf, level); buf.append("\"" + arg_name + "\":{");
	bool first = true;
	for (auto pr : arg_obj) {
		if (!first) buf.append(","); else first = false;

		s(buf, format(pr.first), 0); buf.append(":"); s(buf, pr.second, 0);
	}
	tab(buf, level); buf.append("}");
}
}
#endif
