#ifndef SHARED_MUTEX_H
#define SHARED_MUTEX_H

#include <thread>
#include <mutex>

class shared_mutex
{
private:
	mutable std::mutex	m_shared_mutex;
	mutable std::mutex	m_unique_mutex;
	mutable std::mutex	m_count_mutex;

	mutable unsigned int m_count;

public:
	shared_mutex();

	void lock() const;
	void unlock() const;
	void lock_shared() const;
	void unlock_shared() const;
};

template <typename T>
class shared_lock
{
private:
	const T&	m_mutex;

public:
	shared_lock(const T& mutex) : m_mutex(mutex)
	{
		m_mutex.lock_shared();
	}

	~shared_lock()
	{
		m_mutex.unlock_shared();
	}
};

template <typename T>
class unique_lock
{
private:
	const T&	m_mutex;

public:
	unique_lock(const T& mutex) : m_mutex(mutex)
	{
		m_mutex.lock();
	}

	~unique_lock()
	{
		m_mutex.unlock();
	}
};

#endif
