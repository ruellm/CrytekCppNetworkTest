#pragma once

#include "Packet.h"
#include "ISocketBase.h"

#define MAX_BUFFER_LEN 512

namespace PacketReceiver
{
	PacketPtr Receive(SocketPtr& socket);
}