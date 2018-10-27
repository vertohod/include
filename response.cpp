#include "response.h"

response::response()
{
	http_proto::set_status(S204);
}

bool response::parse()
{
	return http_proto::parse(false);
}

wptr_cstr response::to_raw(bool include_data)
{
	return http_proto::to_raw(false, include_data);
}

method_t response::method()
{
	return http_proto::method();
}

wptr_cstr response::path()
{
	return http_proto::path();
}

ver_t response::ver()
{
	return http_proto::ver();
}

status_t response::status()
{
	return http_proto::status();
}

wptr_str response::raw()
{
	return http_proto::raw();
}

wptr_str response::data()
{
	return http_proto::data();
}

wptr_post response::post(const std::string& key)
{
	return http_proto::post(key);
}

void response::set_method(method_t method)
{
	http_proto::set_method(method);
}

void response::set_path(const std::string& path)
{
	http_proto::set_path(path);
}

void response::set_ver(ver_t ver)
{
	http_proto::set_ver(ver);
}

void response::set_status(status_t status)
{
	http_proto::set_status(status);
}

void response::add_header(const std::string& name, const std::string& val)
{
	http_proto::add_header(sptr_cstr(new std::string(name)), sptr_cstr(new std::string(val)));
}

void response::add_get(const std::string& name, const std::string& val)
{
	http_proto::add_get(sptr_cstr(new std::string(name)), sptr_cstr(new std::string(val)));
}

void response::add_post(const std::string& name, const std::string& ct, const std::string& val)
{
	http_proto::add_post(sptr_cstr(new std::string(name)), sptr_cstr(new std::string(ct)), sptr_cstr(new std::string(val)));
}

void response::add_post(const std::string& name, const std::string& val)
{
	http_proto::add_post(sptr_cstr(new std::string(name)), sptr_cstr(new std::string(val)));
}

void response::add_cookie(const std::string& name, const std::string& val)
{
	http_proto::add_cookie(sptr_cstr(new std::string(name)), sptr_cstr(new std::string(val)));
}

