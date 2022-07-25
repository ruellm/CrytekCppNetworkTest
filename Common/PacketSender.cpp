#include "PacketSender.h"
#include "Packet.h"

#include <iostream>
#include <thread>

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

	bool Wait(SocketPtr& socket)
	{
		int attempt = 0;
		bool terminate = false;

		while (!socket->IsWriteReady(&terminate))
		{
			if (terminate)
				return false;

			// delay to prevent CPU hog/spinning
			std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_THREAD_DELAY));

			if (++attempt >= WAIT_MAX_RETRY)
				return false;
		}

		return true;
	}
}