#include "WindowsSocket.h"
#include <stdexcept>
#include <string>


void CWindowsSocket::Initialize()
{
	// Initialize Winsock
	int addrInfo = WSAStartup(MAKEWORD(2, 2), &m_wsaData);
	if (addrInfo != 0) {
		WSACleanup();
		throw std::runtime_error("Windows Socket not available");
	}
}

void CWindowsSocket::Shutdown()
{
	WSACleanup();
}

WSADATA CWindowsSocket::m_wsaData;

CWindowsSocket::CWindowsSocket(SOCKET_FD handle)
{
	SetHandle(handle);
}

CWindowsSocket::~CWindowsSocket()
{
	Disconnect();
}

bool CWindowsSocket::CreateAsServer(int port)
{
	struct addrinfo *addrInfo = NULL;
	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	std::string PORT = std::to_string(port);
	int result = getaddrinfo(NULL, PORT.c_str(), &hints, &addrInfo);
	if (result != 0) 
	{
		printf("getaddrinfo failed with error: %d\n", result);
		WSACleanup();
		return false;
	}

	// Create a SOCKET for connecting to server
	m_handle = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
	if (m_handle == INVALID_SOCKET) 
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(addrInfo);
		WSACleanup();
		return false;
	}

	// Setup the TCP listening socket
	result = bind(m_handle, addrInfo->ai_addr, (int)addrInfo->ai_addrlen);
	if (result == SOCKET_ERROR) 
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(addrInfo);
		closesocket(m_handle);
		WSACleanup();
		return false;
	}

	result = listen(m_handle, SOMAXCONN);
	if (result == SOCKET_ERROR) 
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(m_handle);
		WSACleanup();
		return false;
	}

	freeaddrinfo(addrInfo);

	return true;
}

SocketPtr CWindowsSocket::Accept()
{
	// Accept a client socket
	SOCKET clientFd = accept(m_handle, NULL, NULL);
	if (clientFd == INVALID_SOCKET) 
	{
		printf("accept failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return nullptr;
	}

	return std::move(std::make_shared<CWindowsSocket>(clientFd));
}

bool CWindowsSocket::Connect(const std::string& address, int port)
{
	struct addrinfo *result = nullptr, *ptr = nullptr, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	std::string PORT = std::to_string(port);
	int res = getaddrinfo(address.c_str(), PORT.c_str(), &hints, &result);
	if (res != 0) 
	{
		printf("getaddrinfo failed with error: %d\n", res);
		WSACleanup();
		return false;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) 
	{
		// Create a SOCKET for connecting to server
		m_handle = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

		if (m_handle == INVALID_SOCKET) 
		{
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return false;
		}

		// Connect to server.
		res = connect(m_handle, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (res == SOCKET_ERROR) 
		{
			closesocket(m_handle);
			m_handle = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);
	return res == 0;
}

void CWindowsSocket::Disconnect()
{
	if (m_handle != INVALID_SOCKET)
	{
		closesocket(m_handle);
		m_handle = INVALID_SOCKET;
	}
}

int CWindowsSocket::Read(void* ptr, int size)
{
	int result = recv(m_handle, (char*)ptr, size, 0);
	if (result == 0)
		printf("Connection closed\n");
	else if (result < 0)
		printf("recv failed with error: %d\n", WSAGetLastError());

	return result;
}

int CWindowsSocket::Write(void* ptr, int size)
{
	// Send an initial buffer
	int result = send(m_handle, (const char*)ptr, size, 0);
	if (result == SOCKET_ERROR) 
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(m_handle);
	}

	return result;
}
