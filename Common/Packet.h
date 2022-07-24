#pragma once

#include <string>
#include <memory>

#define WAIT_THREAD_DELAY 5								/// 5 milliseconds  
#define WAIT_MAX_RETRY	((120*1000)/WAIT_THREAD_DELAY)  /// 2 minute timeout wait for socket to be available for read/write in milliseconds

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