#pragma once

#include "ISocketBase.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iostream>

class CPosixSocket : public ISocketBase
{
public:
	CPosixSocket(SOCKET_FD handle = -1);
	~CPosixSocket();

	virtual bool CreateAsServer(int port) override;
	virtual std::shared_ptr<ISocketBase> Accept() override;
	virtual bool Connect(const std::string& address, int port) override;
	virtual void Disconnect() override;
	virtual int Read(void* ptr, int size) override;
	virtual int Write(void* ptr, int size) override;

private:
	struct sockaddr_in m_address;

};