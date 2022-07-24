#pragma once

#include <memory>
#include <vector>
#include <string>

#ifdef WIN32
#include <winsock2.h>
typedef SOCKET SOCKET_FD;
#else
typedef int SOCKET_FD;
#endif

class SocketBase
{
public:
	virtual bool CreateAsServer(int port) = 0;
	virtual std::shared_ptr<SocketBase> Accept() = 0;
	virtual bool Connect(const std::string& address, int port) = 0;
	virtual void Disconnect() = 0;
	virtual int Read(void* ptr, int size) = 0;
	virtual int Write(void* ptr, int size) = 0;

	bool IsReadReady(bool* terminate);
	bool IsWriteReady(bool* terminate);
	void UnBlock();

	inline void SetHandle(SOCKET_FD handle) {
		m_handle = handle;
	}

	inline SOCKET_FD handle() const {
		return m_handle;
	}

protected:
	SOCKET_FD m_handle;
};

using SocketPtr = std::shared_ptr<SocketBase>;
using SocketList = std::vector<SocketPtr>;
