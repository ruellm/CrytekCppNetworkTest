#include "PacketSender.h"
#include <iostream>

namespace PacketSender
{
	bool Send(IPacketBase* packet, SocketPtr& socket)
	{
		char* buffer = nullptr;
		size_t len = 0;
		buffer = packet->Serialize(&len);

		// write the packet
		int res = socket->Write(buffer, (int)len);
		if (res <= 0)
			return false;
		
		delete[] buffer;
		return true;
	}
}