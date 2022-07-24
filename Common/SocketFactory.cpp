#include "SocketFactory.h"
#ifdef WIN32
#include "WindowsSocket.h"
#else
#include "PosixSocket.h"
#endif

namespace SocketFactory
{
	std::shared_ptr<SocketBase> Create()
	{
#ifdef WIN32
		CWindowsSocket::Initialize();
		return std::make_shared<CWindowsSocket>();
#else
		return std::make_shared<CPosixSocket>();
#endif
	}

	void Destroy()
	{
#ifdef WIN32
		CWindowsSocket::Shutdown();
#endif
	}
}

