#include <cstring>
#include "PacketStringMessage.h"
#include "ByteHandler.h"

SPacketStringMessage::SPacketStringMessage()
{
	header.type = PacketType::StringMessage;
}

char* SPacketStringMessage::Serialize(size_t* len)
{
	size_t datasize = message.size() + from.size() + sizeof(size_t) + sizeof(size_t);
	size_t buffer_size = sizeof(SPacketHeader) + datasize;
	char* buffer = new char[buffer_size];
	char* pBuffer = buffer;

	// copy over the packet details
	SPacketHeader* bufhdr = (SPacketHeader*)(pBuffer);
	*bufhdr = header;
	bufhdr->type = header.type;
	bufhdr->size = header.size = datasize;

	pBuffer += sizeof(SPacketHeader);
	
	// write the from size
	size_t fromIdSize = from.size();
	std::memcpy(pBuffer, (const void*)&fromIdSize, sizeof(size_t));
	pBuffer += sizeof(size_t);

	// write data
	std::memcpy(pBuffer, (const void*)from.c_str(), fromIdSize);
	pBuffer += fromIdSize;
	
	// write data
	size_t dataSize = message.size();
	std::memcpy(pBuffer, (const void*)&dataSize, sizeof(size_t));
	pBuffer += sizeof(size_t);

	std::memcpy(pBuffer, (const void*)message.c_str(), dataSize);
	pBuffer += dataSize;
	
	*len = buffer_size;

	return buffer;
}

void SPacketStringMessage::Deserialized(char* buffer, int len)
{
	SPacketHeader* bufhdr = (SPacketHeader*)(buffer);
	header.type = bufhdr->type;
	header.size = bufhdr->size;
	char* pBuffer = buffer;

	pBuffer += sizeof(SPacketHeader);
	size_t fromSize = ByteHandler::ExtractSize_t(pBuffer);
	pBuffer += sizeof(size_t);

	char* fromBuffer = new char[fromSize + 1];
	std::memcpy(fromBuffer, pBuffer, fromSize);
	fromBuffer[fromSize] = '\0';
	from = std::string(fromBuffer);
	pBuffer += fromSize;

	size_t dataSize = ByteHandler::ExtractSize_t(pBuffer);
	pBuffer += sizeof(size_t);

	char* msg = new char[dataSize + 1];
	std::memcpy(msg, pBuffer, dataSize);
	msg[dataSize] = '\0';
	message = std::string(msg);

	delete[] msg;
	delete[] fromBuffer;
}