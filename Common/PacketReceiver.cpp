#include "PacketReceiver.h"
#include "Packet.h"
#include "Common/PacketBuilder.h"
#include <cstring>
#include <thread>

namespace PacketReceiver
{
	int WaitRead(SocketPtr socket, char* buff, size_t size)
	{
		char temp[MAX_BUFFER_LEN];
		char* p = buff;

		size_t len = 0;
		int target = (int) size;
		int ct = target;
		do
		{
			bool terminate = false;
			if (socket->IsReadReady(&terminate))
			{
				int l = socket->Read(temp, ct);
				if (l <= 0)
					return -1;

				memcpy(p, temp, l);
				p += l;
				len += l;
				ct -= l;
			}
			else
			{
				if (terminate)
					return -1;
			}

		} while (len != target);

		return (int)len;
	}

	PacketPtr Receive(SocketPtr& socket)
	{
		char temp[MAX_BUFFER_LEN];
		int len = WaitRead(socket, temp, sizeof(SPacketHeader));

		// client connection is already closed or lost break the loop
		if (len <= 0)
			return nullptr;

		SPacketHeader* bufhdr = (SPacketHeader*)(temp);
		size_t dataSize = bufhdr->size;

		char* buffer = new char[dataSize + sizeof(SPacketHeader)];

		//2.copy header to buffer
		std::memcpy(buffer, temp, sizeof(SPacketHeader));

		//3. load data
		int dlen = WaitRead(socket, temp, (int)dataSize);
		if (dlen != dataSize) 
		{
			delete buffer;
			return nullptr;
		}

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