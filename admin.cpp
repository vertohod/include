#include "admin.h"

path_not_supported::path_not_supported(const std::string& arg_what) : m_what(arg_what)
{
}

const char* path_not_supported::what() const noexcept
{
	return m_what.c_str();
}
