#include "PacketBuilder.h"
#include "PacketStringMessage.h"
#include "PacketIdentity.h"

#include <memory>

namespace PacketBuilder
{
	PacketPtr Build(char* buffer, size_t len)
	{
		SPacketHeader* hdr = (SPacketHeader*)(buffer);
		PacketPtr packet = nullptr;

		switch (hdr->type)
		{
		case PacketType::StringMessage:
			{
				packet = std::make_unique<SPacketStringMessage>();
				break;
			}
		case PacketType::Identity:
			{
				packet = std::make_unique<SPacketIdentiy>();
				break;
			}
		}

		if (packet)
			packet->Deserialized(buffer, (int)len);
	
		return std::move(packet);
	}
}