#pragma once

#include "Packet.h"


struct SBroadCastPeerMessage : public IPacketBase
{
	std::string peerOriginate;
	char* buffer;
	size_t dataSize;

	SBroadCastPeerMessage();
	~SBroadCastPeerMessage();

	void Set(const std::string& id, char* data, size_t len);
	virtual char* Serialize(size_t* len) override;
	virtual void Deserialized(char* buffer, int len) override;
};