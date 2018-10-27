#include "serialization.h"

namespace serialization
{

void begin(std::string& buf)
{
	buf.append("{");
}

void end(std::string& buf)
{
	buf.append("}");
}

void comma(std::string& buf)
{
	buf.append(",");
}

void enter(std::string& buf)
{
	buf.append("\n");
}

void tab(std::string& buf, int level)
{
	for (auto count = 0; count < level; ++count) {
		buf.append("\t");
	}
}

template <>
void s<std::string>(std::string& buf, const std::string& arg_obj, int level)
{
	buf += "\"" + escape(format(arg_obj)) + "\"";
}

template <>
void s<char>(std::string& buf, const char& arg_obj, int level)
{
	buf += "\"" + escape(format(arg_obj)) + "\"";
}

std::string escape(const std::string& arg_value)
{
	std::string res;

    for (auto lt : arg_value) {
		switch (lt) {
			case '\\': res += "\\\\"; break;
			case '"':  res += "\\\""; break;
//			case '/':  res += "\\/"; break;
			case '\b': res += "\\b"; break;
			case '\f': res += "\\f"; break;
			case '\n': res += "\\n"; break;
			case '\r': res += "\\r"; break;
			case '\t': res += "\\t"; break;
			default: res += lt; break;
		}
    }

    return res;
}

}
