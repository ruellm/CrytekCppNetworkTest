#pragma once

#include "Thread/ThreadPool.h"
#include "Config/PeersConfig.h"
#include "Common/ISocketBase.h"

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

	CEchoDistributedServer(const SConfig& config);
	~CEchoDistributedServer();

	void Run( const PeersList& peers);

private:
	void BroadCastMessage(char* buffer, int len, const SocketPtr& from, bool peer = false);
	void ConnectToPeers(const PeersList& peers);
	void RemoveFromList(const SocketPtr& socket);
	void ProcessClient(SocketPtr& socket, bool peer = false);
	void AddSocket(SocketPtr& socket);

private:
	CThreadPool m_pool;
	SocketList m_clientList;
	const SConfig& m_config;
	std::mutex m_mutex;
};