#pragma once

#include <string>

enum class PacketType : int
{
	StringMessage,
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
	virtual char* Serialize(size_t* len) = 0;
	virtual void Deserialized(char* buffer, int len) = 0;
};

struct SPacketStringMessage : public IPacketBase
{
	std::string message;
	SPacketStringMessage();
	void Set(const std::string& message);

	virtual char* Serialize(size_t* len) override;
	virtual void Deserialized(char* buffer, int len) override;
};

using PacketPtr = std::unique_ptr<IPacketBase>;