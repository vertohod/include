#include "shared_mutex.h"

shared_mutex::shared_mutex() : m_count(0) {}

void shared_mutex::lock() const
{
	m_shared_mutex.lock();
	m_unique_mutex.lock();	
}

void shared_mutex::unlock() const
{
	m_unique_mutex.unlock();	
	m_shared_mutex.unlock();
}

void shared_mutex::lock_shared() const
{
	std::lock_guard<std::mutex> lock1(m_shared_mutex);
	std::lock_guard<std::mutex> lock2(m_count_mutex);

	if (m_count == 0) m_unique_mutex.lock();
	++m_count;
}

void shared_mutex::unlock_shared() const
{
	std::lock_guard<std::mutex> lock(m_count_mutex);

	--m_count;
	if (m_count == 0) m_unique_mutex.unlock();
}
