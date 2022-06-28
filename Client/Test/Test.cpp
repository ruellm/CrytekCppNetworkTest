#include "Test.h"
#include "Common/PacketBuilder.h"
#include "Common/PacketStringMessage.h"
//#include "Common/BroadCastPeerMessage.h"

#include <assert.h>     /* assert */

namespace Test
{
	void TestSerialize()
	{
		SPacketStringMessage message;
		message.message = ("The quick brown fox jumps over the lazy dog");
		message.from = "client_1";

		char* buffer = nullptr;
		size_t len = 0;

		buffer = message.Serialize(&len);
		auto result = PacketBuilder::Build(buffer, (int)len);
		SPacketStringMessage* derived = dynamic_cast<SPacketStringMessage*>(result.get());
		
		assert(message.header.type == derived->header.type);
		assert(message.header.size == derived->header.size);
		assert(message.message.compare(derived->message) == 0);
		assert(message.from.compare(derived->from) == 0);

#if 0
		SBroadCastPeerMessage broadcast;
		broadcast.buffer = buffer;
		broadcast.dataSize = len;
		broadcast.peerOriginate = "Peer 1";
		broadcast.from = "client_2";

		char* bbuffer = broadcast.Serialize(&len);
		result = PacketBuilder::Build(bbuffer, (int)len);
		SBroadCastPeerMessage* broadcastDerived = dynamic_cast<SBroadCastPeerMessage*>(result.get());

		assert(broadcast.from.compare(broadcastDerived->from) == 0);

		result = PacketBuilder::Build(broadcastDerived->buffer, (int)broadcastDerived->dataSize);
		SPacketStringMessage* derived_2 = dynamic_cast<SPacketStringMessage*>(result.get());

		assert(derived_2->header.type == derived->header.type);
		assert(derived_2->header.size == derived->header.size);
		assert(derived_2->message.compare(derived->message) == 0);
#endif
		delete[] buffer;
	}
}
