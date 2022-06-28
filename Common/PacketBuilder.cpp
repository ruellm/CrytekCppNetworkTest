#include "PacketBuilder.h"
#include "PacketStringMessage.h"
//#include "BroadCastPeerMessage.h"
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
#if 0
		case PacketType::BroadCastPeer:
			{
				 packet = std::make_unique<SBroadCastPeerMessage>();
				 break;
			}
#endif
		case PacketType::Identity:
		{
			packet = std::make_unique<SPacketIdentiy>();
			break;
		}
		}

		if (packet)
			packet->Deserialized(buffer, len);
	
		return std::move(packet);
	}
}