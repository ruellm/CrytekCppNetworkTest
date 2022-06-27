#include "Test.h"
#include "../Network/PacketBuilder.h"

#include <assert.h>     /* assert */

namespace Test
{
	void TestSerialize()
	{
		SPacketStringMessage message;
		message.Set("The quick brown fox jumps over the lazy dog");

		char* buffer = nullptr;
		int len = 0;

		buffer = message.Serialize(&len);
		auto result = PacketBuilder::Build(buffer, len);
		SPacketStringMessage* derived = dynamic_cast<SPacketStringMessage*>(result.get());
		
		assert(message.header.type == derived->header.type);
		assert(message.header.size == derived->header.size);
		assert(message.message.compare(derived->message) == 0);

		delete[] buffer;
	}
}
