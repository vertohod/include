#ifndef REQUEST_H
#define REQUEST_H

#include "http_proto.h"

class request : protected http_proto
{
public:
	request();

	bool parse();
	sptr_cstr to_raw(bool include_data = true);

    method_t method() const;
    sptr_cstr path() const;
    ver_t ver() const;
    status_t status() const;

    sptr_str raw();
    sptr_str data();
	void set_raw(sptr_str);

    template <typename T>
    T header(const std::string& key, const T& val) const
	{
		return http_proto::header<T>(key, val);
	}

    template <typename T>
    T get(const std::string& key, const T& val) const
	{
		return http_proto::get<T>(key, val);
	}

    wptr_post post(const std::string& key) const;

    template <typename T>
    T cookie(const std::string& key, const T& val) const
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
