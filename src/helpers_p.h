#ifndef HELPERS_P_H
#define HELPERS_P_H

#include <QtCore/QtGlobal>
#include <unistd.h>
#include <fcntl.h>
#include "qt4compat.h"

Q_DECL_HIDDEN inline qint64 safe_read(int fd, void* data, qint64 maxlen)
{
	qint64 ret = 0;
	do {
		ret = ::read(fd, data, maxlen);
	} while (ret == -1 && errno == EINTR);
	return ret;
}

Q_DECL_HIDDEN inline qint64 safe_write(int fd, const void* data, qint64 len)
{
	qint64 ret = 0;
	do {
		ret = ::write(fd, data, len);
	} while (ret == -1 && errno == EINTR);
	return ret;
}

Q_DECL_HIDDEN inline void my_close(int& fd)
{
	if (fd != -1) {
		::close(fd);
		fd = -1;
	}
}

Q_DECL_HIDDEN inline int set_cloexec(int fd)
{
	return fcntl(fd, F_SETFD, FD_CLOEXEC);
}

Q_DECL_HIDDEN inline int make_nonblocking(int fd)
{
	int res = fcntl(fd, F_GETFL);
	if (res != -1) {
		return fcntl(fd, F_SETFL, res | O_NONBLOCK);
	}

	return -1;
}

#endif // HELPERS_P_H
