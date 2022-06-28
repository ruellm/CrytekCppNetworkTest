#pragma once

#include "Packet.h"

struct SPacketStringMessage : public IPacketBase
{
	std::string message;
	SPacketStringMessage();

	virtual char* Serialize(size_t* len) override;
	virtual void Deserialized(char* buffer, int len) override;
};