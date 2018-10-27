#ifndef STAT_BASE_H
#define STAT_BASE_H

#include <boost/thread/thread.hpp>
#include <unordered_map>
#include <string>
#include <mutex>
#include <ctime>

#include <unistd.h>
#include <time.h>

#include "serialization.h"
#include "types.h"
#include "util.h"

#define ASKTIME     10		// 10 секунд
#define MICROSEC    1000000
#define NANOSEC     1000000000

namespace std {
    template<>
    class hash<boost::thread::id>
    {
    public:
        size_t operator()(const boost::thread::id&) const;
    };
}

struct tick_data
{
	size_t		m_count;
	size_t		m_total;
	double		m_min;
	double		m_max;
	double		m_rps;

	tick_data();
};

namespace serialization
{
template <>
void s<tick_data>(std::string& buf, const tick_data& obj, int level);
}

typedef std::unordered_map<boost::thread::id, tick_data> tick_data_t;

template <typename T>
struct tick
{
private:
	static std::mutex m_mutex;
	static tick_data_t m_data;

public:
	static void t()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		++m_data[boost::this_thread::get_id()].m_count;
	}

	static tick_data get()
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		tick_data res;

		for (auto& data_pr : m_data) {
			data_pr.second.m_total += data_pr.second.m_count;
			data_pr.second.m_rps = static_cast<double>(data_pr.second.m_count) / ASKTIME;

			data_pr.second.m_min = (data_pr.second.m_max == 0.0) ? data_pr.second.m_rps : (data_pr.second.m_min > data_pr.second.m_rps ? data_pr.second.m_rps : data_pr.second.m_min); 
			data_pr.second.m_max = (data_pr.second.m_max < data_pr.second.m_rps) ? data_pr.second.m_rps : data_pr.second.m_max;

			res.m_count += data_pr.second.m_count;
			res.m_total += data_pr.second.m_total;
			res.m_rps += data_pr.second.m_rps;
			res.m_min += data_pr.second.m_min;
			res.m_max += data_pr.second.m_max;

			data_pr.second.m_count = 0;
		}

		return res;
	}

	static void clear()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		
		for (auto& data_pr : m_data) {
			data_pr.second.m_count = 0;
			data_pr.second.m_min = 0.0;
			data_pr.second.m_max = 0.0;
			data_pr.second.m_rps = 0.0;
		}
	}
};

struct interval_data
{
	size_t		m_count;	
	double		m_interval;
	double		m_total;
	double		m_min;
	double		m_max;
	double		m_avg;

	bool		m_flag_start;
	timespec	m_time;

	interval_data();
};

namespace serialization
{
template <>
void s<interval_data>(std::string& buf, const interval_data& obj, int level);
}

typedef std::unordered_map<boost::thread::id, interval_data> interval_data_t;

template <typename T>
struct interval
{
private:
	static std::mutex m_mutex;
	static interval_data_t m_data;

public:
	interval()
	{
		start();
	}

	~interval()
	{
		finish();
	}

	static void start()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		boost::thread::id id = boost::this_thread::get_id();
		m_data[id].m_flag_start = true;
#ifdef CLOCK_REALTIME
		clock_gettime(CLOCK_REALTIME, &(m_data[id].m_time));
#endif
#ifdef CLOCK_REALTIME_PRECISE 
		clock_gettime(CLOCK_REALTIME_PRECISE, &(m_data[id].m_time));
#endif
	}

	static void finish()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		timespec time_temp;
#ifdef CLOCK_REALTIME
		clock_gettime(CLOCK_REALTIME, &time_temp);
#endif
#ifdef CLOCK_REALTIME_PRECISE 
		clock_gettime(CLOCK_REALTIME_PRECISE, &time_temp);
#endif
		auto& data = m_data[boost::this_thread::get_id()];
		if (data.m_flag_start) {
			data.m_flag_start = false;
			++data.m_count;
			data.m_interval += static_cast<double>(NANOSEC + time_temp.tv_nsec - data.m_time.tv_nsec) / NANOSEC + (time_temp.tv_sec - data.m_time.tv_sec) - 1;
		}
	}

	static interval_data get(double correction = 0.0)
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		interval_data res;
		size_t count_local = 0;

		for (auto& data_pr : m_data) {

			if (data_pr.second.m_count != 0) {

				++count_local;

				data_pr.second.m_avg = data_pr.second.m_interval / data_pr.second.m_count - correction;
				data_pr.second.m_min = (data_pr.second.m_max == 0.0) ? data_pr.second.m_avg : (data_pr.second.m_min > data_pr.second.m_avg ? data_pr.second.m_avg : data_pr.second.m_min);
				data_pr.second.m_max = data_pr.second.m_max < data_pr.second.m_avg ? data_pr.second.m_avg : data_pr.second.m_max;
				data_pr.second.m_total += data_pr.second.m_interval;

				res.m_count += data_pr.second.m_count;
				res.m_avg += data_pr.second.m_avg;
			}

			res.m_min += data_pr.second.m_min;
			res.m_max += data_pr.second.m_max;
			res.m_total += data_pr.second.m_total;

			data_pr.second.m_count = 0;
			data_pr.second.m_interval = 0.0;
		}

		if (count_local > 0) res.m_avg /= count_local;

		if (m_data.size() > 0) {
			res.m_min /= m_data.size();
			res.m_max /= m_data.size();
		}

		return res; 
	}

	static void clear()
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		for (auto& data_pr : m_data) {
			data_pr.second.m_count = 0;
			data_pr.second.m_interval = 0.0;
			data_pr.second.m_min = 0.0;
			data_pr.second.m_max = 0.0;
			data_pr.second.m_avg = 0.0;
		}
	}
};

struct size_data
{
	size_t	m_count;
	size_t	m_min;
	size_t	m_max;
	size_t	m_size;
	time_t	m_time;

	size_data();
};

namespace serialization
{
template <>
void s<size_data>(std::string& buf, const size_data& obj, int level);
}

typedef std::unordered_map<boost::thread::id, size_data> size_data_t;

template <typename T>
struct size
{
private:
	static std::mutex m_mutex;
	static size_data_t m_data;

public:
	static void s(size_t value)
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		auto& data = m_data[boost::this_thread::get_id()];
		data.m_size = value;
		data.m_time = time(NULL);
		++data.m_count;
	}

	static size_data get()
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		size_data res;
		time_t time_temp = 0;

		for (auto& data_pr : m_data) {
			if (data_pr.second.m_count != 0) {
				data_pr.second.m_min = (data_pr.second.m_max == 0) ? data_pr.second.m_size : (data_pr.second.m_min > data_pr.second.m_size ? data_pr.second.m_size : data_pr.second.m_min); 
				data_pr.second.m_max = (data_pr.second.m_max < data_pr.second.m_size) ? data_pr.second.m_size : data_pr.second.m_max;
			}

			// Выбераем данные из того потока, в котором они менялись позже
			if (time_temp < data_pr.second.m_time) {
				res.m_count = data_pr.second.m_count;
				res.m_min = data_pr.second.m_min;
				res.m_max = data_pr.second.m_max;
				res.m_size = data_pr.second.m_size;

				time_temp = data_pr.second.m_time;
			}

			data_pr.second.m_count = 0;
		}

		return res;
	}

	static void clear()
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		for (auto& data_pr : m_data) {
			data_pr.second.m_count = 0;
			data_pr.second.m_min = data_pr.second.m_size;
			data_pr.second.m_max = data_pr.second.m_size;
		}
	}
};

////////////////////////////////////////////////////////////

struct correction;
typedef interval<correction> correction_interval;

class stat_base
{
private:
	double			m_correction;

public:
	stat_base();

	virtual sptr_str create();
	virtual void clear();
	double corr();
};

template <typename T>
class profile 
{
private:
	std::mutex m_mutex;
	boost::thread m_thread;

	T m_stat_obj;
	std::string m_stat;

public:
	profile() {
		boost::thread thread(boost::bind(&profile::thread, this));
		m_thread.swap(thread);

		clear();
	}

	std::string get()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		return m_stat;
	}

	void clear()
	{
		m_stat_obj.clear();
	}

private:
	profile(const profile&) = delete;
	profile& operator=(const profile&) = delete;

	void create()
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		m_stat.clear();

		{
			using namespace serialization;

			begin(m_stat); enter(m_stat);
			m_stat.append(*m_stat_obj.create());
			tab(m_stat, 1); m_stat.append("\"time\":" + format(time(NULL))); enter(m_stat);
			end(m_stat); enter(m_stat);
		}
	}

	void thread()
	{
		while (true) {
			usleep(ASKTIME * MICROSEC);

			create();

			time_t rawtime; struct tm *tm_temp;
			time(&rawtime);
			tm_temp = localtime(&rawtime);

			// Раз в час сбрасываем минимумы и максимумы
			if (tm_temp->tm_min == 0 && tm_temp->tm_sec < ASKTIME * 1.5) {
				clear();
			}
		}
	}
};

#define STAT_TICK(name, name_type) struct name; typedef tick<name> name_type;
#define STAT_TICK_DEF(name_type) template<>std::mutex name_type::m_mutex {}; template<>tick_data_t name_type::m_data {};
#define STAT_INTERVAL(name, name_type) struct name; typedef interval<name> name_type;
#define STAT_INTERVAL_DEF(name_type) template<>std::mutex name_type::m_mutex {}; template<>interval_data_t name_type::m_data {};
#define STAT_SIZE(name, name_type) struct name; typedef size<name> name_type;
#define STAT_SIZE_DEF(name_type) template<>std::mutex name_type::m_mutex {}; template<>size_data_t name_type::m_data {};

#endif
