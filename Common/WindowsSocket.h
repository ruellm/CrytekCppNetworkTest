#pragma once

#include "SocketBase.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

class CWindowsSocket : public SocketBase
{
public:
	CWindowsSocket(SOCKET_FD handle = INVALID_SOCKET);
	~CWindowsSocket();

	virtual bool CreateAsServer(int port) override;
	virtual SocketPtr Accept() override;
	virtual bool Connect(const std::string& address, int port) override;
	virtual void Disconnect() override;
	virtual int Read(void* ptr, int size) override;
	virtual int Write(void* ptr, int size) override;

	static void Initialize();
	static void Shutdown();

private:
	static WSADATA m_wsaData;
};