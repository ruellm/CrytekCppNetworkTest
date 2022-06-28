#pragma once

#include "Packet.h"
#include "ISocketBase.h"

namespace PacketSender
{
	void Send(IPacketBase* packet, SocketPtr& socket );
}