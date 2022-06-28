#include "BroadCastPeerMessage.h"
#include "ByteHandler.h"

SBroadCastPeerMessage::SBroadCastPeerMessage() : buffer(nullptr)
{
	header.type = PacketType::BroadCastPeer;
}

SBroadCastPeerMessage::~SBroadCastPeerMessage()
{
	if (buffer)
		delete[] buffer;
	buffer = nullptr;
}

void SBroadCastPeerMessage::Set(const std::string& id, char* data, size_t len)
{
	buffer = new char[len];
	memcpy(buffer, data, len);
	dataSize = len;
	peerOriginate = id;
}

char* SBroadCastPeerMessage::Serialize(size_t* len)
{
	size_t idSize = peerOriginate.size();
	size_t wholeSize = dataSize + idSize + sizeof(size_t);
	size_t buffer_size = sizeof(SPacketHeader) + wholeSize;
	char* tmpBuffer = new char[buffer_size];
	char* pBuffer = tmpBuffer;

	// copy over the packet details
	SPacketHeader* bufhdr = (SPacketHeader*)(pBuffer);
	*bufhdr = header;
	bufhdr->type = PacketType::BroadCastPeer;
	bufhdr->size = wholeSize;
	pBuffer += sizeof(SPacketHeader);

	size_t fromIdSize = from.size();
	std::memcpy(pBuffer, (const void*)&fromIdSize, sizeof(size_t));
	pBuffer += sizeof(size_t);

	std::memcpy(pBuffer, (const void*)from.c_str(), fromIdSize);
	pBuffer += fromIdSize;

	// write the peer id size
	std::memcpy(pBuffer, (const void*)&idSize, sizeof(size_t));
	pBuffer += sizeof(size_t);

	std::memcpy(pBuffer, (const void*)peerOriginate.c_str(), idSize);
	pBuffer += idSize;

	// write the data size
	std::memcpy(pBuffer, (const void*)&dataSize, sizeof(size_t));
	pBuffer += sizeof(size_t);

	// write actual data
	std::memcpy(pBuffer, (const void*)buffer, dataSize);

	*len = buffer_size;

	return tmpBuffer;
}

void SBroadCastPeerMessage::Deserialized(char* buffer_, int len)
{
	SPacketHeader* bufhdr = (SPacketHeader*)(buffer_);
	header.type = bufhdr->type;
	header.size = bufhdr->size;

	buffer += sizeof(SPacketHeader);
	size_t fromSize = ByteHandler::ExtractSize_t(buffer);
	buffer += sizeof(size_t);

	char* fromBuffer = new char[fromSize + 1];
	std::memcpy(fromBuffer, buffer, fromSize);
	fromBuffer[header.size] = '\0';
	from = std::string(fromBuffer);
	delete[] fromBuffer;
	buffer += fromSize;

	buffer_ += sizeof(SPacketHeader);
	size_t idSize = ByteHandler::ExtractSize_t(buffer_);
	buffer_ += sizeof(size_t);

	char* peerId = new char[idSize + 1];
	std::memcpy(peerId, buffer_, idSize);
	peerId[idSize] = '\0';
	peerOriginate = std::string(peerId);
	delete peerId;

	buffer_ += idSize;
	size_t dataSize = ByteHandler::ExtractSize_t(buffer_);
	buffer_ += sizeof(size_t);

	buffer = new char[dataSize];
	std::memcpy(buffer, buffer_, dataSize);
}