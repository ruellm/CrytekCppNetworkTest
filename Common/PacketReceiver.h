#pragma once

#include "Packet.h"
#include "SocketBase.h"

#define MAX_BUFFER_LEN 512

namespace PacketReceiver
{
	PacketPtr Receive(SocketPtr& socket);
	bool Wait(SocketPtr& socket);
}