#include "PacketReceiver.h"
#include "Packet.h"
#include "Common/PacketBuilder.h"
#include <cstring>
#include <thread>

namespace PacketReceiver
{
	PacketPtr Receive(SocketPtr& socket)
	{	
		char temp[MAX_BUFFER_LEN];

		//1. read header first
		int len = socket->Read(temp, sizeof(SPacketHeader));

		// client connection is already closed or lost break the loop
		if (len <= 0)
			return nullptr;

		SPacketHeader* bufhdr = (SPacketHeader*)(temp);
		size_t dataSize = bufhdr->size;

		char* buffer = new char[dataSize + sizeof(SPacketHeader)];

		//2.copy header to buffer
		std::memcpy(buffer, temp, sizeof(SPacketHeader));

		//3. load data
		int dlen = socket->Read(temp, (int)dataSize);
		if (dlen != dataSize)
			return nullptr;

		std::memcpy(buffer + sizeof(SPacketHeader), temp, dataSize);

		len += (int)bufhdr->size;

		auto packet = PacketBuilder::Build(buffer, len);
		delete buffer;

		return std::move(packet);
	}

	bool Wait(SocketPtr& socket)
	{
		int attempt = 0;
		bool terminate = false;
		while (!socket->IsReadReady(&terminate))
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