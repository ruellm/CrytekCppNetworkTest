#include "SocketBase.h"


bool SocketBase::IsReadReady()
{
	FD_SET writeSet;
	FD_SET readSet;
	DWORD total;

	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);
	FD_SET(m_handle, &readSet);

	if ((total = select(0, &readSet, &writeSet, NULL, NULL)) == SOCKET_ERROR)
	{
		printf("select() returned with error %d\n", WSAGetLastError());
		return false;
	}

	if (!FD_ISSET(m_handle, &readSet))
	{
		printf("Socket not ready for read\n");
		return false;
	}

	return true;
}

bool SocketBase::IsWriteReady()
{
	FD_SET writeSet;
	FD_SET readSet;
	DWORD total;

	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);
	FD_SET(m_handle, &writeSet);

	if ((total = select(0, &readSet, &writeSet, NULL, NULL)) == SOCKET_ERROR)
	{
		printf("select() returned with error %d\n", WSAGetLastError());
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
	ioctlsocket(m_handle, FIONBIO, &mode);
}