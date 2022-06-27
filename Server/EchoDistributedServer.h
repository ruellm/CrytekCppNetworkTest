#pragma once

#include "ThreadPool.h"
#include "Config/ServerConfig.h"
#include "Config/PeersConfig.h"
#include "ISocketBase.h"

class CEchoDistributedServer
{
public:
	CEchoDistributedServer(const SServerConfig& config);
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
	const SServerConfig& m_config;
	std::mutex m_mutex;
};