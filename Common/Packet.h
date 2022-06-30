#pragma once

#include <string>
#include <memory>

enum class PacketType : int
{
	StringMessage,
	BroadCastPeer,
	Identity
	//For expansion ...
};

struct SPacketHeader
{
	PacketType type;
	size_t size = 0;
};

struct IPacketBase
{
	SPacketHeader header;
	std::string from;

	virtual char* Serialize(size_t* len) = 0;
	virtual void Deserialized(char* buffer, int len) = 0;
};

using PacketPtr = std::shared_ptr<IPacketBase>;