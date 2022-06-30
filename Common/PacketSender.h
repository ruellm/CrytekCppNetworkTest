#pragma once

#include "Packet.h"
#include "ISocketBase.h"

namespace PacketSender
{
	bool Send(IPacketBase* packet, SocketPtr& socket );
}