#include "SocketFactory.h"
#include "WindowsSocket.h"

namespace SocketFactory
{
	std::shared_ptr<ISocketBase> Create()
	{
#ifdef WIN32
		CWindowsSocket::Initialize();
		return std::make_shared<CWindowsSocket>();
#else
		return nullptr;
#endif
	}

	void Destroy()
	{
#ifdef WIN32
		CWindowsSocket::Shutdown();
#endif
	}
}

