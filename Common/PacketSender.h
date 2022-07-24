#pragma once

#include "Packet.h"
#include "SocketBase.h"

namespace PacketSender
{
	bool Send(IPacketBase* packet, SocketPtr& socket );
	bool Wait(SocketPtr& socket);
}