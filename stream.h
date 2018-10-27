#ifndef STREAM_H
#define STREAM_H

#include <string>
#include "format.h"

namespace sp 
{

class stream
{
	bool m_create_data;
	std::string &m_data;

public:
	stream(std::string &arStr);
	stream();
	~stream();
	
	template<typename T>
	stream& operator<<(const T& val)
	{
		m_data.append(format(val)); 
		return *this;
	}

	stream& operator>>(std::string &arStr);

	const std::string& data() const;

private:
	stream(const stream&);
	stream& operator=(const stream&);
}; 

}

#endif

