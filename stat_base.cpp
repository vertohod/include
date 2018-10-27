#include "stat_base.h"
#include "log.h"

size_t std::hash<boost::thread::id>::operator()(const boost::thread::id& obj) const
{
    return boost::hash<boost::thread::id>()(obj);
}

tick_data::tick_data() : m_count(0), m_total(0), m_min(0.0), m_max(0.0), m_rps(0.0) {}

interval_data::interval_data() : m_count(0), m_interval(0.0), m_total(0.0),
	m_min(0.0), m_max(0.0), m_avg(0.0), m_flag_start(false)
{}

size_data::size_data() : m_count(0), m_min(0), m_max(0), m_size(0), m_time(0) {}

////////////////////////////////////////////////////////////

template<>
std::mutex correction_interval::m_mutex {};
template<>
interval_data_t correction_interval::m_data {};

stat_base::stat_base() : m_correction(0.0) {}

sptr_str stat_base::create()
{
	interval_data data_temp;
	for (size_t count = 0; count < 10; ++count) {
		correction_interval::start();
		correction_interval::finish();
		data_temp = correction_interval::get();
	}
	m_correction = data_temp.m_min;

	return sptr_str();
}

void stat_base::clear()
{
	correction_interval::clear();
}

double stat_base::corr()
{
	return m_correction;
}

namespace serialization
{

template <>
void s<tick_data>(std::string& buf, const tick_data& obj, int level)
{
    begin(buf); enter(buf);
	s(buf, "rps", obj.m_rps, level + 1); comma(buf); enter(buf);
	s(buf, "max", obj.m_max, level + 1); comma(buf); enter(buf);
	s(buf, "min", obj.m_min, level + 1); comma(buf); enter(buf);
	s(buf, "count", obj.m_count, level + 1); comma(buf); enter(buf);
	s(buf, "total", obj.m_total, level + 1); enter(buf);
    tab(buf, level); end(buf); comma(buf); enter(buf);
}

template <>
void s<interval_data>(std::string& buf, const interval_data& obj, int level)
{
    begin(buf); enter(buf);
	s(buf, "avg", obj.m_avg, level + 1); comma(buf); enter(buf);
	s(buf, "max", obj.m_max, level + 1); comma(buf); enter(buf);
	s(buf, "min", obj.m_min, level + 1); comma(buf); enter(buf);
	s(buf, "count", obj.m_count, level + 1); comma(buf); enter(buf);
	s(buf, "total", obj.m_total, level + 1); enter(buf);
    tab(buf, level); end(buf); comma(buf); enter(buf);
}

template <>
void s<size_data>(std::string& buf, const size_data& obj, int level)
{
    begin(buf); enter(buf);
	s(buf, "size", obj.m_size, level + 1); comma(buf); enter(buf);
	s(buf, "max", obj.m_max, level + 1); comma(buf); enter(buf);
	s(buf, "min", obj.m_min, level + 1); comma(buf); enter(buf);
	s(buf, "count", obj.m_count, level + 1); enter(buf);
    tab(buf, level); end(buf); comma(buf); enter(buf);
}

} // end of namespace serialization
