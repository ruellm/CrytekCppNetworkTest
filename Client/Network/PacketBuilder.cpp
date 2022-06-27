#include "PacketBuilder.h"

namespace PacketBuilder
{
	PacketPtr Build(char* buffer, int len)
	{
		SPacketHeader* hdr = (SPacketHeader*)(buffer);

		switch (hdr->type)
		{
		case PacketType::StringMessage:
			{
				auto packet = std::make_unique<SPacketStringMessage>();
				packet->Deserialized(buffer, len);
				return std::move(packet);
			}
		}

		return nullptr;
	}
}