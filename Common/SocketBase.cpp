#include "SocketBase.h"

#if defined(_WIN32)
#define GETSOCKETERRNO() (WSAGetLastError())
#else
/* According to POSIX.1-2001 */
#include <sys/select.h>

/* According to earlier standards */
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#define GETSOCKETERRNO() (errno)
#define SOCKET_ERROR -1
#endif

bool SocketBase::IsReadReady(bool* terminate)
{
	fd_set readSet;
	FD_ZERO(&readSet);

	FD_SET(m_handle, &readSet);
	int maxsock =0;

	struct timeval tv;
	tv.tv_sec = 120;
	tv.tv_usec = 0;

#ifndef WIN32
	maxsock = m_handle+1;
#endif

	if ((select(maxsock, &readSet, NULL, NULL, &tv)) == SOCKET_ERROR)
	{
		auto error = GETSOCKETERRNO();

#ifdef WIN32
		if (error == WSAENOTSOCK)
			*terminate = true;
#else
		if (error == ENOTSOCK)
			*terminate = true;
#endif

		printf("select() returned with error %d\n", error);
		return false;
	}

	if (!FD_ISSET(m_handle, &readSet))
	{
		printf("Socket not ready for read\n");
		return false;
	}

	return true;
}

bool SocketBase::IsWriteReady(bool* terminate)
{
	fd_set writeSet;

	FD_ZERO(&writeSet);
	FD_SET(m_handle, &writeSet);

	int maxsock =0;

	struct timeval tv;
	tv.tv_sec = 120;
	tv.tv_usec = 0;

#ifndef WIN32
	maxsock = m_handle+1;
#endif

	if ((select(maxsock, NULL, &writeSet, NULL, &tv)) == SOCKET_ERROR)
	{
		auto error = GETSOCKETERRNO();

#ifdef WIN32
		if (error == WSAENOTSOCK)
			*terminate = true;
#else
		if (error == ENOTSOCK)
			*terminate = true;
#endif
		printf("select() returned with error %d\n", error);
		return false;
	}

	if (!FD_ISSET(m_handle, &writeSet))
	{
		printf("Socket not ready for write\n");
		return false;
	}

	return true;
}

void SocketBase::UnBlock()
{
	u_long mode = 1;  // 1 to enable non-blocking socket
#ifdef WIN32
	ioctlsocket(m_handle, FIONBIO, &mode);
#else
	ioctl(m_handle, FIONBIO, &mode);
#endif
}