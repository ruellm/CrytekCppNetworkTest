#include "PacketSender.h"
#include "Packet.h"

#include <iostream>
#include <thread>

namespace PacketSender
{
	bool Send(IPacketBase* packet, SocketPtr& socket)
	{
		int written = 0;

		char* buffer = nullptr;
		size_t len = 0;
		buffer = packet->Serialize(&len);
		char* p = buffer;
		size_t current = len;

		do {
			bool terminate = false;
			if (socket->IsWriteReady(&terminate)) {
				int res = socket->Write(p, (int)current);
				if (res <= 0)
					goto exit;

				p += res;
				written += res;
				current -= res;
			}
			else {
				if (terminate)
					goto exit;
			}
		} while (written != len);
	exit:
		delete[] buffer;
		return true;
	}
}