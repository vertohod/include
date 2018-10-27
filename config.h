#ifndef CONFIG_H
#define CONFIG_H

#include "config_base.h"

class config : public config_base
{
public:
	std::string path_to_stdout;

public:
	config();
	config(const std::string& arPath);

	virtual void call_back();
};

#endif
