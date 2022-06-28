#pragma once

#include "PacketStringMessage.h"

struct SPacketIdentiy : public SPacketStringMessage
{
	SPacketIdentiy() = default;

	SPacketIdentiy(const std::string& id) {
		header.type = PacketType::Identity;
		message = id;
	}
};