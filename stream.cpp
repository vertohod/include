#include "stream.h"

namespace sp 
{

stream::stream(std::string &arStr) : m_create_data(false), m_data(arStr)
{
}

stream::stream() : m_create_data(true), m_data(*new std::string())
{
}

stream::~stream()
{ 
	if (m_create_data) delete &m_data;
}

stream& stream::operator>>(std::string &arStr)
{
	arStr = m_data;
	return *this;
}
	
const std::string& stream::data() const
{
	return m_data;
}

stream::stream(const stream&) : m_create_data(true), m_data(*new std::string())
{
}

stream& stream::operator=(const stream&)
{
	return *this;
}

}

