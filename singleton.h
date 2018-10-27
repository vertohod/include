#ifndef SINGLETON_H
#define SINGLETON_H

#include <stdexcept>
#include <memory>

template <typename T>
class singleton
{
private:
	static std::unique_ptr<T> m_ptr;

public:
	static void init(T* ptr)
	{
		m_ptr.reset(ptr);
	}

	static T& instance()
	{
		if (!m_ptr) throw std::runtime_error("Invalid pointer");
		return *m_ptr;
	}
};

template <typename T>
std::unique_ptr<T> singleton<T>::m_ptr;

#endif
