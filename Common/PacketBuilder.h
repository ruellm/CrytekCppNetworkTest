#pragma once

#include "Packet.h"
#include <memory>

namespace PacketBuilder
{
	PacketPtr Build(char* buffer, size_t len);
}