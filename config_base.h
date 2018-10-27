#ifndef CONFIG_BASE_H
#define CONFIG_BASE_H

#include <unordered_map>
#include <string>

#include "format.h"
#include "log.h"

class config_base;

typedef std::unordered_map<std::string, std::string> map_str_t;
typedef std::unordered_map<std::string, map_str_t> data_map_t;

namespace magic
{
	template <typename OPTION>
	void proc_option(data_map_t& m_data, map_str_t& m_info, const std::string& arName, const std::string& arInfo, OPTION& arOption, bool arMandatory)
	{
		auto it = m_data.find(arName);

		if (m_data.end() == it) {
			if (arMandatory) {
				throw std::runtime_error("option " + arName + " was not set");
			} else {
				lo::l(lo::WARNING) << "option " << arName << " was not set";
				return;
			}
		}
		
		OPTION value_temp = format<OPTION>(it->second.begin()->second);
		if (arOption != value_temp) {
			arOption = value_temp;
			lo::l() << "option " << arName << " = " << value_temp << "\t# " << arInfo;
		}

		m_info.insert(std::make_pair(arName, arInfo));
	}

	template <>
	void proc_option<map_str_t>(data_map_t& m_data, map_str_t& m_info, const std::string& arName, const std::string& arInfo, map_str_t& arOption, bool arMandatory);

} // end of magic

class options
{
private:
	map_str_t	m_info;
	data_map_t	m_data;

public:
	options();

	template <typename T>
	options& operator()(const std::string& arName, const std::string& arInfo, T& arOption, bool arMandatory = true)
	{
		magic::proc_option(m_data, m_info, arName, arInfo, arOption, arMandatory);
		return *this;
	}

	void check(); // Проверка лишних параметров в файле

private:
	options(const options&);
	options& operator=(const options&);
	void init(const std::string& arData);
	void parse(const std::string& arData);
	void parse_cmd(int argc, char* argv[]);

	friend class config_base;
};

class config_base
{
public:
	std::string		path_to_config;
	size_t			sleep_update_config;

private:
	options			m_options;

public:
	void init(const std::string& arPath);
	void parse_cmd(int argc, char* argv[]);
	void update();

protected:
	options& get_options();
	void check();
	virtual void call_back();

public:
	config_base();
	config_base(const std::string& arPath);

private:
	config_base(const config_base&);
	config_base& operator=(const config_base&);
	void init();

private:
};

#endif

