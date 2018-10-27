#ifndef UNIXSOCK_H
#define UNIXSOCK_H

#include <pwd.h>
#include <string>
#include <vector>

namespace chatd
{

inline char *cpystrn(char *dst, const char *src, size_t dst_size);
int get_unix_socket(const std::string& path, const std::string& user, const std::string& group, const std::string& mode);

} // end of namespace chatd
#endif
