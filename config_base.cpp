#include <algorithm>
#include <fstream>
#include <string>

#include "config_base.h"
#include "util.h"
#include "log.h"

namespace magic
{

template <>
void proc_option<map_str_t>(data_map_t& m_data, map_str_t& m_info, const std::string& arName, const std::string& arInfo, map_str_t& arOption, bool arMandatory)
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

	if (it->second.size() != arOption.size() ||
		!std::equal(it->second.begin(), it->second.end(), arOption.begin()))
	{
		arOption = it->second;
	}

	m_info.insert(std::make_pair(arName, arInfo));
}

} // end of magic

options::options()
{
}

options::options(const options&)
{
}

options& options::operator=(const options&)
{
	return *this;
}

void options::check()
{
	for (const auto& opt_pair : m_data) {
		auto info_it = m_info.find(opt_pair.first);
		if (info_it == m_info.end()) throw std::runtime_error("unknown option in configuration file: " + opt_pair.first);
	}
}

void options::init(const std::string& arData)
{
	m_data.clear();
	m_info.clear();

	parse(arData);
}

void options::parse(const std::string& arData)
{
	auto lines = util::split(arData, "\n");				

	for (auto line : *lines) {
		size_t pos_hash = line.find("#");
		if (pos_hash != std::string::npos) {
			line = line.substr(0, pos_hash);
		}
		if (line.empty()) continue;

		size_t pos_equel = line.find("=");
		if (pos_equel != std::string::npos) {
			auto key = util::trim(line.substr(0, pos_equel));
			auto val = util::trim(line.substr(pos_equel + 1, line.size() - pos_equel - 1));

			auto pos_colon = key.find(":");
			if (pos_colon != std::string::npos) {
				auto key_map1 = util::trim(key.substr(0, pos_colon));
				auto key_map2 = util::trim(key.substr(pos_colon + 1, key.size() - pos_colon - 1));
				m_data[key_map1][key_map2] = val;
			} else {
				m_data[key][""] = val;
			}
		}
	}
}

void options::parse_cmd(int argc, char* argv[])
{
	std::string str_in;
	std::string command_line;

	for (int counter = 0; counter < argc; ++counter) {
		std::string one_option(argv[counter]);

		command_line += one_option + " ";

		if (one_option.size() > 2) {
			if (one_option[0] == '-' && one_option[1] == '-') {

				one_option = one_option.substr(2, one_option.size() - 2);
				str_in += one_option + "\n";	
			}
		}
	}

	lo::l() << "command line: " << command_line;
		
	parse(str_in);
}

// config_base - methods

void config_base::init(const std::string& arPath)
{
	path_to_config.assign(arPath);
	init();
}

void config_base::init()
{
	if (path_to_config.empty()) return;

	std::ifstream	ifs;
	std::string		str_in;

	lo::l(lo::DEBUG) << "config file: " << path_to_config;

	ifs.open(path_to_config.c_str(), std::ifstream::in);
	if (ifs.fail()) throw std::runtime_error("failed to open config file");

	for (char ch = ifs.get(); ifs.good(); ch = ifs.get()) {
		str_in.push_back(ch);
	}

	ifs.close();

	if (str_in.empty()) return;

	m_options.init(str_in);
}

void config_base::parse_cmd(int argc, char* argv[])
{
	m_options.parse_cmd(argc, argv);

	// Возможно в командной строке будет параметр,
	// указывающий на конфигурационный файл.
	// Сразу попытаемся его обработать.
	config_base::call_back();
	init();
	call_back();
}

options& config_base::get_options()
{
	return m_options;
}

config_base::config_base()
{
}

config_base::config_base(const std::string& arPath) : path_to_config(arPath)
{
	init();
}

config_base::config_base(const config_base&)
{
}

config_base& config_base::operator=(const config_base&)
{
	return *this;
}

void config_base::update()
{
	init();
	call_back();
}

void config_base::call_back()
{
	auto& op = get_options();

	op  
		("path-to-config", "Path to config file", path_to_config)
	;
}

void config_base::check()
{
	m_options.check();
}	
