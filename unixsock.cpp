#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include <exception>
#include <stdexcept>
#include "unixsock.h"
#include "util.h"
#include "format.h"

namespace chatd
{

inline char *cpystrn(char *dst, const char *src, size_t dst_size)
{
	char *d, *end;
	if (!dst_size) return dst;
	d = dst;
	end = dst + dst_size - 1;
	for (; d < end; ++d, ++src) {
		if (!(*d = *src)) {
			return d;
		}
	}
	*d = '\0';
	return d;
}

int get_unix_socket(const std::string& path, const std::string& user, const std::string& group, const std::string& mode)
{
	struct sockaddr_un sa_un;

	memset(&sa_un, 0, sizeof(sa_un));
	cpystrn(sa_un.sun_path, path.c_str(), sizeof(sa_un.sun_path));
	sa_un.sun_family = AF_UNIX;

	int sock = socket(reinterpret_cast<struct sockaddr*>(&sa_un)->sa_family, SOCK_STREAM, 0);
	if (sock < 0) throw std::runtime_error("Can't create UNIX-socket");

	int flags = 1;

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flags, sizeof(flags));
	unlink(sa_un.sun_path);

	if (0 > bind(sock, reinterpret_cast<struct sockaddr*>(&sa_un), sizeof(sockaddr_un))) {
		throw std::runtime_error("Can't bind UNIX-socket");
	}

	// -------------------------------------------------------------
	if (!geteuid()) {
		util::passwd	pwd;
		util::group		grp;

		pwd = util::getpwnam(user);
		if (pwd.pw_name.empty()) throw std::runtime_error("Can't get user's information");

		grp = util::getgrnam(group);
		if (grp.gr_name.empty()) throw std::runtime_error("Can't get group's information");

		uid_t uid = pwd.pw_uid;
		gid_t gid = grp.gr_gid;

		if (0 > chown(sa_un.sun_path, uid, gid)) {
			throw std::runtime_error("Can't install right (chown)");
		}
	}
	// ------------------------------------------------------------

	int socket_mode = strtoul(mode.c_str(), 0, 8);

	if (0 > chmod(path.c_str(), socket_mode)) {
		throw std::runtime_error("Can't change socket's mode");
	}

	if (0 > listen(sock, -1)) {
		throw std::runtime_error("Can't begin to listen a socket");
	}

	return sock;
}

} // end of namespace chatd

