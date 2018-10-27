#include "request.h"

request::request()
{
}

bool request::parse()
{
	return http_proto::parse(true);
}

sptr_cstr request::to_raw(bool include_data)
{
	return http_proto::to_raw(true, include_data);
}

method_t request::method() const
{
	return http_proto::method();
}

sptr_cstr request::path() const
{
	return http_proto::path();
}

ver_t request::ver() const
{
	return http_proto::ver();
}

status_t request::status() const
{
	return http_proto::status();
}

sptr_str request::raw()
{
	return http_proto::raw();
}

sptr_str request::data()
{
	return http_proto::data();
}

void request::set_raw(sptr_str raw)
{
	http_proto::set_raw(raw);
}

wptr_post request::post(const std::string& key) const
{
	return http_proto::post(key);
}

void request::set_method(method_t method)
{
	http_proto::set_method(method);
}

void request::set_path(const std::string& path)
{
	http_proto::set_path(path);
}

void request::set_ver(ver_t ver)
{
	http_proto::set_ver(ver);
}

void request::set_status(status_t status)
{
	http_proto::set_status(status);
}

void request::add_header(const std::string& name, const std::string& val)
{
	http_proto::add_header(sptr_cstr(new std::string(name)), sptr_cstr(new std::string(val)));
}

void request::add_get(const std::string& name, const std::string& val)
{
	http_proto::add_get(sptr_cstr(new std::string(name)), sptr_cstr(new std::string(val)));
}

void request::add_post(const std::string& name, const std::string& ct, const std::string& val)
{
	http_proto::add_post(sptr_cstr(new std::string(name)), sptr_cstr(new std::string(ct)), sptr_cstr(new std::string(val)));
}

void request::add_post(const std::string& name, const std::string& val)
{
	http_proto::add_post(sptr_cstr(new std::string(name)), sptr_cstr(new std::string(val)));
}

void request::add_cookie(const std::string& name, const std::string& val)
{
	http_proto::add_cookie(sptr_cstr(new std::string(name)), sptr_cstr(new std::string(val)));
}

