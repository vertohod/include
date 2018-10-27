#ifndef RESPONSE_H
#define RESPONSE_H

#include "http_proto.h"

class response : protected http_proto
{
public:
	response();

	bool parse();
	wptr_cstr to_raw(bool include_data = true);

    method_t method();
    wptr_cstr path();
    ver_t ver();
    status_t status();

    wptr_str raw();
    wptr_str data();

    template <typename T>
    T header(const std::string& key, const T& val)
	{
		return http_proto::header<T>(key, val);
	}

    template <typename T>
    T get(const std::string& key, const T& val)
	{
		return http_proto::get<T>(key, val);
	}

    wptr_post post(const std::string& key);

    template <typename T>
    T cookie(const std::string& key, const T& val)
	{
		return http_proto::cookie(key, val);
	}

    void set_method(method_t);
    void set_path(const std::string&);
    void set_ver(ver_t);
    void set_status(status_t);

    void add_header(const std::string&, const std::string&);
    void add_get(const std::string&, const std::string&);
    void add_post(const std::string&, const std::string&, const std::string&);
    void add_post(const std::string&, const std::string&);
    void add_cookie(const std::string&, const std::string&);
};

#endif
