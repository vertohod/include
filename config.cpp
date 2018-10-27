#include "config.h"

config::config()
{
}

config::config(const std::string& arPath) : config_base(arPath)
{
}

void config::call_back()
{
	config_base::call_back();

	auto& op = get_options();

	// Здесь будут распологаться стандартные параметры
	op  
		("path-to-stdout", "Path to stdout", path_to_stdout)
	;
}
