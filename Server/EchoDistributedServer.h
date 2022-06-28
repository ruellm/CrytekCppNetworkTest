#pragma once

#include "Thread/ThreadPool.h"
#include "Config/PeersConfig.h"
#include "Common/ISocketBase.h"
#include "Common/Packet.h"

using SocketMapId = std::map<std::string, SocketPtr>;

class CEchoDistributedServer
{
public:
	struct SConfig
	{
		std::string id;				// server identity
		int port = 0;				// server port
		int numOfThreads = 100;		// the number of threads in the thread pool
		bool expand = false;		// will the server expand and create more thread if it runs out
	};

	struct SConnectionGroup
	{
		
	};

	CEchoDistributedServer(const SConfig& config);
	~CEchoDistributedServer();

	void Run( const PeersList& peers);

private:
	void BroadCastMessage(char* buffer, size_t len, 
		const std::string& origin, const std::string& sender);
	
	void ConnectToPeers(const PeersList& peers, SocketPtr& socket);
	void RemoveFromList(const SocketPtr& socket);
	void ProcessClient(SocketPtr& socket);

	void AddClient(SocketPtr& socket, const std::string& id);
	void ConfirmIdentity(SocketPtr& socket, bool peers);
	const std::string GetSocketId(const SocketPtr& socket);
	void RunSockets(SocketPtr& socket, bool peers);
	bool IsPeer(const std::string& id);
	PacketPtr ValidateIdentity(SocketPtr& ptr);
	bool BasicHandShake(const std::string& id);

private:
	CThreadPool m_pool;
	SConnectionGroup m_group[3];
	const SConfig& m_config;
	std::mutex m_mutex;
	SocketMapId m_clients;

};