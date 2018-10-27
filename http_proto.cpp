#include <algorithm>

#include "http_proto.h"
#include "stream.h"
#include "util.h"
#include "log.h"

post_field::post_field()
{
	m_value = std::make_pair(m_value_hidden->begin(), m_value_hidden->end());
}

post_field::post_field(const sptr_cstr& name, const sptr_cstr& content_type, const sptr_cstr& file_name, const sptr_cstr& value) :
	m_name(name), m_content_type(content_type), m_file_name(file_name), m_value_hidden(value)
{
	m_value = std::make_pair(m_value_hidden->begin(), m_value_hidden->end());
	m_size = m_value_hidden->size();
}

post_field::post_field(const sptr_cstr& name, const sptr_cstr& content_type, const sptr_cstr& file_name, value_t value) :
	m_name(name), m_content_type(content_type), m_file_name(file_name), m_value(value)
{
	m_size = value.second - value.first;
}

wptr_cstr post_field::name() const
{
	return m_name;
}

wptr_cstr post_field::ct() const
{
	return m_content_type;
}

wptr_cstr post_field::file() const
{
	return m_file_name;
}

size_t post_field::size() const
{
	return m_size;
}

value_t post_field::val_pr() const
{
	return m_value;
}

ver_t::ver_t() : byte1(0), byte2(0) {}

ver_t::ver_t(const std::string& str)
{
	static const std::string pr("HTTP/");

	auto pos = str.find(pr);
	if (pos != std::string::npos) {

		auto res = util::split(str.substr(pos + pr.length(), str.length() - pos - pr.length()), ".");

		if (res->size() > 0)
			byte1 = format<unsigned char>(res->at(0));

		if (res->size() > 1)
			byte2 = format<unsigned char>(res->at(1));
	}
}

bool ver_t::operator<(ver_t arg_obj)
{
    if (byte1 < arg_obj.byte1) return true;
    if (byte1 == arg_obj.byte1) {
        if (byte2 < arg_obj.byte2) return true;
    }
    return false;
}

std::string ver_t::to_str() const
{
    sp::stream st;
    st << "HTTP/" << byte1 << "." << byte2;
    return st.data();
}

http_proto::http_proto() :
	m_status(SUNKNOWN), m_state(METHOD),
	m_current(0), m_cvalue(0), m_flag_2n(false) {}


method_t http_proto::method() const
{
	return m_method;
}

wptr_cstr http_proto::path() const
{
	return m_path;
}

ver_t http_proto::ver() const
{
	return m_ver;
}

status_t http_proto::status() const
{
	return m_status;
}

size_t http_proto::content_length() const
{
	return m_content_length;
}

sptr_str http_proto::raw()
{
	return m_raw;
}

sptr_str http_proto::data()
{
	return m_data;
}

void http_proto::set_raw(sptr_str raw)
{
	m_raw = raw;
}

wptr_post http_proto::post(const std::string& key) const
{
	auto it = m_post.find(key);
	if (it == m_post.end()) return sptr_post();
	return it->second;
}

void http_proto::set_method(method_t method)
{
	m_method = method;
}

void http_proto::set_path(const std::string& path)
{
	m_path = path;
}

void http_proto::set_ver(ver_t ver)
{
	m_ver = ver;
}

void http_proto::set_status(status_t status)
{
	m_status = status;
}

void http_proto::add_header(const sptr_cstr& key, const sptr_cstr& val)
{
	m_header.insert(std::make_pair(key, val));
}

void http_proto::add_get(const sptr_cstr& key, const sptr_cstr& val)
{
	m_get.insert(std::make_pair(key, val));
}

void http_proto::add_post(const sptr_cstr& name, const sptr_cstr& value)
{
	add_post(name, sptr_cstr(), value);
}

void http_proto::add_post(const sptr_cstr& name, const sptr_cstr& ct, const sptr_cstr& value)
{
	m_post.insert(std::make_pair(name, sptr_post(new post_field(name, ct, sptr_cstr(), value))));
}

void http_proto::add_cookie(const sptr_cstr& key, const sptr_cstr& val)
{
	lo::l(lo::DEBUG) << "add_cookie, " << *key << "=" << *val;

	m_cookie.insert(std::make_pair(key, val));
}

bool http_proto::parse(bool req)
{
	if (!req && m_state == METHOD) m_state = VERSION;

	for (size_t count = m_current; count < m_raw->size(); ++count) {
		auto b = m_raw->at(count);

		switch (m_state) {
			case METHOD: {
				if (b == ' ') {
					if (count - m_current > 0) {
						m_method = method_from_str(m_raw->substr(m_current, count - m_current));
						m_state = PATH;
						m_current = count + 1;
					} else return false;
				}
				break;
			}
			case PATH: {
				if (b == ' ' || b == '?') {
					if (count - m_current > 0) {
						m_path = m_raw->substr(m_current, count - m_current);

						if (b == '?') m_state = GETOPTIONS;
						else m_state = VERSION;

						m_current = count + 1;
					} else return false;
				}
				break;
			}
			case GETOPTIONS: {
				if (b == ' ') {
					if (count - m_current > 0) {
						parse_urlencoded(m_current, count, boost::bind(&http_proto::add_get, this, _1, _2));
					}
					m_current = count + 1;
				}
			}
			case VERSION: {
				if ((req && (b == '\n' || b == '\r')) || (!req && b == ' ')) {
					if (count - m_current > 0) {
						m_ver = ver_t(m_raw->substr(m_current, count - m_current));
						m_state = req ? HEADER : STATUS;
						m_current = count + 1;
					} else return false;
				}
				break;
			}
			case STATUS: {
				if (b == '\n') {

					m_state = HEADER;
					m_current = count + 1;

				} else if (b == ' ' && m_status == SUNKNOWN) {

					if (count - m_current > 0) {
						m_status = status_from_int(format<size_t>(m_raw->substr(m_current, count - m_current)));
					} else return false;

				}
				break;
			}
			case HEADER: {
				if (b == '\n' || b == '\r') {

					if (b == '\n' && m_flag_2n) {

						if (req && m_method == GET) return true;

						m_state = BODY;
						m_current = count + 1;

						continue;
					}
					if (b == '\n') m_flag_2n = true;

					if (m_cvalue  > m_current + 2) {

						sptr_str hf_name(new std::string(m_raw->substr(m_current, m_cvalue - m_current - 2)));
						sptr_cstr hf_value(m_raw->substr(m_cvalue, count - m_cvalue));

						lo::l(lo::DEBUG) << "Header, " << *hf_name << ": " << *hf_value;
						std::transform(hf_name->begin(), hf_name->end(), hf_name->begin(), ::tolower);

						m_header.insert(std::make_pair(hf_name, hf_value));

						if (*hf_name == "cookie") {
							parse_urlencoded(m_cvalue, count, boost::bind(&http_proto::add_cookie, this, _1, _2), ';', false);
						}
					}
					m_current = count + 1;

				} else if (b == ':' && m_cvalue < m_current) {
					m_cvalue = count + 2;
				} else {
					m_flag_2n = false;
					continue;
				}
				break;
			}
			case BODY: {
				if (b == '\n' || b == '\r') {
					m_current = count + 1;
					continue;
				}

				m_content_length = header("content-length", 0);
				if (m_content_length > 0 && m_content_length > m_raw->size() - m_current) return false;

				lo::l(lo::DEBUG) << "Content-Length: " << m_content_length;

				if (req && m_method == POST) parse_body();
				else m_data = sptr_str(new std::string(m_raw->substr(m_current, m_raw->size() - m_current)));

				return true;
			}
		}
	}
	return true;
}

void http_proto::parse_body()
{
	static std::string ct_multipart("multipart/form-data");
	static std::string ct_urlencoded("application/x-www-form-urlencoded");
	static std::string ct_boundary("boundary=");
	
	sptr_cstr content_type = header("content-type", sptr_cstr());

	if (content_type->empty()) {
		parse_urlencoded(m_current, m_raw->size(), boost::bind(&http_proto::add_post, this, _1, _2));
		return;
	}

	bool mult_flag = true;
	bool urle_flag = true;

	for (size_t count = 0; count < content_type->size(); ++count) {

		// multipart/form-data
		if (mult_flag && count < ct_multipart.size() && content_type->at(count) == ct_multipart.at(count)) {
			if (count + 1 == ct_multipart.size()) {

				auto pos = content_type->find(ct_boundary);
				if (pos != std::string::npos) {
					std::string boundary = content_type->substr(pos + ct_boundary.size(), content_type->size() - pos - ct_boundary.size());

					parse_multipart(boundary);
					return;
				}
			}
		} else mult_flag = false;

		// application/x-www-form-urlencoded
		if (urle_flag && count < ct_urlencoded.size() && content_type->at(count) == ct_urlencoded.at(count)) {
			if (count + 1 == ct_urlencoded.size()) {
				parse_urlencoded(m_current, m_raw->size(), boost::bind(&http_proto::add_post, this, _1, _2));

				return;
			}
		} else urle_flag = false;
	}

	lo::l(lo::ERROR) << "Content type not supported: " << *content_type;
}

void http_proto::parse_multipart(const std::string& boundary)
{
    static std::string name("name=\"");
    static std::string file_name("filename=\"");
    static std::string content_type_U("Content-Type: ");
    static std::string content_type_L("content-type: ");
    static std::string body_start_label1("\r\n\r\n");
    static std::string body_start_label2("\n\n");

	size_t content_type_start = 0;
	size_t content_type_size = 0;
	size_t name_start = 0;
	size_t name_size = 0;
	size_t filename_start = 0;
	size_t filename_size = 0;
	size_t body_start = 0;
	long body_size = 0;

	enum parse_state {
		HEADER,
		NAME,
		FILENAME,
		CONTENT_TYPE,
		BODY
	} state = HEADER;

	for (m_current += boundary.size() + 2; m_current < m_raw->size(); ++m_current) {

		switch (state) {

			case HEADER: {   

				if (name_size == 0) {
					const size_t size = minBSize(name.size());
					if (!memcmp(&m_raw->at(m_current), name.c_str(), size)) {
						m_current += size;
						name_start = m_current;
						state = NAME;
						continue;
					}
				}
				if (filename_size == 0) {
					const size_t size = minBSize(file_name.size());
					if (!memcmp(&m_raw->at(m_current), file_name.c_str(), size)) {
						m_current += size;
						filename_start = m_current;
						state = FILENAME;
						continue;
					}
				}
				if (content_type_size == 0) {
					const size_t size = minBSize(content_type_U.size());
					if (!memcmp(&m_raw->at(m_current), content_type_U.c_str(), size) ||
						!memcmp(&m_raw->at(m_current), content_type_L.c_str(), size)) {

						m_current += size;
						content_type_start = m_current;
						state = CONTENT_TYPE;
						continue;
					}
				}
				if (body_size == 0) {
					{
						const size_t size = minBSize(body_start_label1.size());
						if (!memcmp(&m_raw->at(m_current), body_start_label1.c_str(), size)) {
							m_current += size;
							body_start = m_current;
							state = BODY;
							continue;
						}
					}
					{
						const size_t size = minBSize(body_start_label2.size());
						if (!memcmp(&m_raw->at(m_current), body_start_label2.c_str(), size)) {
							m_current += size;
							body_start = m_current;
							state = BODY;
							continue;
						}
					}
				}
				continue;
			}

			case NAME: {

				if (m_raw->at(m_current) == '"') {
					name_size = m_current - name_start;
					state = HEADER;
				}
				continue;
			}

			case FILENAME: {

				if (m_raw->at(m_current) == '"') {
					filename_size = m_current - filename_start;
					state = HEADER;
				}
				continue;
			}

			case CONTENT_TYPE: {

				if (m_raw->at(m_current) == '\r' || m_raw->at(m_current) == '\n') {
					content_type_size = m_current - content_type_start;
					--m_current;
					state = HEADER;
				}
				continue;
			}

			case BODY: {

				const size_t size = minBSize(boundary.size());
				if (!memcmp(&m_raw->at(m_current), boundary.c_str(), size)) {

					body_size = m_current - body_start - 2;

					if (body_size < 0) body_size = 0;
					else if (body_size >= 2
							&& m_raw->at(body_start + body_size - 1) == '\n'
							&& m_raw->at(body_start + body_size - 2) == '\r') {

						body_size -= 2;
					}

					if (name_size != 0) {

						sptr_cstr field_name(m_raw->substr(name_start, name_size));
						sptr_cstr field_content_type(m_raw->substr(content_type_start, content_type_size));
						sptr_cstr field_file_name(m_raw->substr(filename_start, filename_size));
						value_t field_value(std::make_pair(m_raw->begin() + body_start, m_raw->begin() + body_start + body_size));

						m_post.insert(std::make_pair(field_name, sptr_post(new post_field(field_name, field_content_type, field_file_name, field_value))));
					}

					m_current += size;
					state = HEADER;
					content_type_start = 0;
					content_type_size = 0;
					name_start = 0;
					name_size = 0;
					filename_start = 0;
					filename_size = 0;
					body_start = 0;
					body_size = 0;
				}
				continue;
			}
		}
	}
}

void http_proto::parse_urlencoded(size_t off_begin, size_t off_end, boost::function<void(sptr_cstr, sptr_cstr)> func, char separator, bool unquote_flag)
{
    size_t name_start = off_begin;
    size_t name_size = 0;
    size_t value_start = 0;
    size_t value_size = 0;

    for (size_t count = off_begin; count < off_end; ++count) {

		if (m_raw->at(count) == ' ') {

			name_start = count + 1;

		} else if (m_raw->at(count) == '=' && value_start == 0) {

            name_size = count - name_start;
            value_start = count + 1;

        } else if (value_start != 0) {

            if (m_raw->at(count) == separator) {

                value_size = count - value_start;

            } else if (count + 1 == off_end) {

                value_size = count - value_start + 1;

            } else continue;

			if (unquote_flag) {
				func(unquote(m_raw->substr(name_start, name_size)), unquote(m_raw->substr(value_start, value_size)));
			} else {
				func(
					sptr_cstr(m_raw->substr(name_start, name_size)),
					sptr_cstr(m_raw->substr(value_start, value_size))
				);
			}

            name_start = count + 1;
            value_start = 0;
        }
    }
}

size_t http_proto::minBSize(size_t size)
{
	return std::min(size, m_raw->size() - m_current);
}

wptr_cstr http_proto::method_to_str(method_t method)
{
	static std::unordered_map<method_t, sptr_cstr> method_list {
		{OPTIONS,	"OPTIONS"},
		{GET,		"GET"},
		{HEAD,		"HEAD"},
		{POST,		"POST"},
		{PUT,		"PUT"},
		{DELETE,	"DELETE"},
		{TRACE,		"TRACE"},
		{CONNECT,	"CONNECT"},
		{PATCH,		"PATCH"}
	};
	static sptr_cstr unknown("UNKNOWN");

	auto it = method_list.find(method);

	return it == method_list.end() ? unknown : it->second;
}

method_t http_proto::method_from_str(const std::string& method)
{
	if (method == "OPTIONS") return OPTIONS;
	else if (method == "GET") return GET;
	else if (method == "HEAD") return HEAD;
	else if (method == "POST") return POST;
	else if (method == "PUT") return PUT;
	else if (method == "DELETE") return DELETE;
	else if (method == "TRACE") return TRACE;
	else if (method == "CONNECT") return CONNECT;
	else if (method == "PATCH") return PATCH;
	else return UNKNOWN;
}

wptr_cstr http_proto::status_to_str(status_t status)
{   
	static std::unordered_map<status_t, sptr_cstr> status_list {
		{S200, "200 OK"},
		{S204, "204 No Content"},
		{S302, "302 Found"},
		{S403, "403 Forbidden"},
		{S404, "404 Not Found"},
		{S405, "405 Method Not Allowed"},
		{S500, "500 Internal Server Error"},
		{S501, "501 Not Implemented"}
	};
	static sptr_cstr empty;

	auto it = status_list.find(status);

    return it == status_list.end() ? empty : it->second;
}

status_t http_proto::status_from_int(size_t status)
{
    switch (status) {
        case 200:
            return S200;
        case 204:
            return S204;
        case 302:
            return S302;
        case 403:
            return S403;
        case 404:
            return S404; 
        case 405:
            return S405;
        case 500:
            return S500;
        case 501:
            return S501;
		default:
			return SUNKNOWN;
    }
    return SUNKNOWN;
}

unsigned char recode_char(char ch)
{
    if (ch >= '0' and ch <= '9')
        return static_cast<unsigned char>(ch) - static_cast<unsigned char>('0');

    if (ch >= 'a' and ch <= 'f')
        return static_cast<unsigned char>(ch) - static_cast<unsigned char>('a') + 10;

    if (ch >= 'A' and ch <= 'F')
        return static_cast<unsigned char>(ch) - static_cast<unsigned char>('A') + 10;

    return 0;
}

uptr_cstr unquote(const std::string& str)
{
    uptr_str res;

    unsigned char nbyte = 0;
    unsigned char ch_temp = 0;

    for (char ch : str) {
        if (ch == '%') {
            nbyte = 1;
        } else if (nbyte == 1) {
            ch_temp = recode_char(ch) << 4;
            nbyte = 2;
        } else if (nbyte == 2) {
            ch_temp |= recode_char(ch);
            *res += static_cast<char>(ch_temp);
            nbyte = 0;
        } else if (ch == '+') *res += ' '; else *res += ch;
    }
    return res;
}

uptr_cstr quote(const std::string& str)
{
	static const std::string table("0123456789ABCDEF");

    uptr_str res;

	for (const auto& ch : str) {

		if ((ch >= '0' and ch <= '9') ||
			(ch >= 'a' and ch <= 'z') ||
			(ch >= 'A' and ch <= 'Z') ||
			ch == '-' || ch == '_' || ch == '.' || ch == '~') {

			*res += ch;

		} else if (ch == ' ') {

			*res += '+';

		} else {

			*res += '%';
			*res += table[static_cast<unsigned char>(ch) >> 4];
			*res += table[static_cast<unsigned char>(ch) & 0x0F];

		}
	}

    return res;
}

wptr_cstr http_proto::to_raw(bool req, bool include_data)
{
	if (req) {
		// Первая строка запроса
		*m_raw += *method_to_str(m_method).lock();
		*m_raw += ' ';

		*m_raw += *m_path;
		if (m_get.size() > 0) {
			// Параметры могут быть уже в path
			auto pos = m_path->find("?");
			if (pos == std::string::npos) *m_raw += '?';
			// В таком случаи добавляем ? и первый параметр из get без &
			bool flag = (pos != std::string::npos);
			for (auto& pr : m_get) {
				if (flag) *m_raw += '&'; else flag = true;
				*m_raw += *quote(*pr.first) + "=" + *quote(*pr.second);
			}
		}
		*m_raw += ' ';
		*m_raw += m_ver.to_str();
		*m_raw += '\n';

	} else {
		// Первая и вторая строка ответа
		*m_raw += m_ver.to_str();
		*m_raw += ' ';
		*m_raw += *status_to_str(m_status).lock();
		*m_raw += '\n';

		*m_raw += "Status: " + *status_to_str(m_status).lock() + '\n';
	}

	// Заголовок
	for (auto& pr : m_header) {
		*m_raw += *pr.first + ": " + *pr.second + '\n';
	}

	// Куки
	if (m_cookie.size() > 0) {
		if (req) {
			*m_raw += "Cookie: ";
		} else {
			*m_raw += "Set-Cookie: ";
		}

		bool flag = false;
		for (auto& pr : m_cookie) {
			if (flag) *m_raw += "; "; else flag = true;
			*m_raw += *pr.first + "=" + *pr.second;
		}

		if (!req) {
			*m_raw += "; expires=" + *expires() + "; secure";
//			*m_raw += "; expires=" + *expires();
		}

		*m_raw += '\n';
	}

	// Область данных
	if (req) {
		std::string data_temp;
		if (m_post.size() > 0) {
			bool flag = false;
			for (auto& pr : m_post) {
				if (flag) data_temp += '&'; else flag = true;
				data_temp += *quote(*pr.first) + "=" + *quote(pr.second->val<std::string>());
			}

			*m_raw += "Content-Length: " + format(data_temp.length());
		}
		*m_raw += "\n\n";
		*m_raw += data_temp;
	} else {
		if (m_data->length() > 0) {
			*m_raw += "Content-Length: " + format(m_data->length());
		}
		*m_raw += "\n\n";

		if (include_data) {
			*m_raw += *m_data;
		}
	}

	return m_raw;
}

uptr_cstr expires()
{   
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    rawtime += 31536000; // Один год

    timeinfo = gmtime(&rawtime);

    char buffer[160];

    strftime(buffer, 160, "%a, %e-%b-%G %T GMT", timeinfo);

    return uptr_cstr(buffer);
}

size_t std::hash<method_t>::operator()(method_t m) const
{
    return std::hash<int>()(static_cast<int>(m));
}

size_t std::hash<status_t>::operator()(status_t s) const
{
    return std::hash<int>()(static_cast<int>(s));
}
