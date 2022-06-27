#include "Packet.h"
#include <cstring>

SPacketStringMessage::SPacketStringMessage()
{
	header.type = PacketType::StringMessage;
}

void SPacketStringMessage::Set(const std::string& message_)
{
	message = message_;
	header.size = message.size();
}

char* SPacketStringMessage::Serialize(size_t* len)
{
	size_t datasize = message.size();
	size_t buffer_size = sizeof(SPacketHeader) + datasize;
	char* buffer = new char[buffer_size];

	// copy over the packet details
	SPacketHeader* bufhdr = (SPacketHeader*)(buffer);
	*bufhdr = header;
	bufhdr->type = PacketType::StringMessage;
	bufhdr->size = datasize;

	// copy over the data
	std::memcpy(buffer + sizeof(SPacketHeader), message.c_str(), datasize);

	*len = buffer_size;

	return buffer;
}

void SPacketStringMessage::Deserialized(char* buffer, int len)
{
	SPacketHeader* bufhdr = (SPacketHeader*)(buffer);
	header.type = bufhdr->type;
	header.size = bufhdr->size;

	char* msg = new char[header.size + 1];
	std::memcpy(msg, buffer + sizeof(SPacketHeader), header.size);
	msg[header.size] = '\0';
	message = std::string(msg);

	delete[] msg;
}