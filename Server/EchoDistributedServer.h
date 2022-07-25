#pragma once

#include "Thread/ThreadPool.h"
#include "Config/PeersConfig.h"
#include "Common/SocketBase.h"
#include "Common/Packet.h"

#include <chrono>
#include<unordered_set>

using SocketMapId = std::map<std::string, SocketPtr>;

class CEchoDistributedServer
{
public:

	enum class PeersConnectType : int
	{
		Duplex,												// server both listens and connects to peers
		Listener,											// server only listens to connections from peers
		Active												// server initiates connection to its peers
	};

	struct SConfig
	{
		std::string id;										// server identity
		int port = 0;										// server port
		int numOfThreads = 100;								// the number of threads in the thread pool
		bool expand = false;								// will the server expand and create more thread if it runs out
		PeersConnectType type = PeersConnectType::Duplex;	// Connection type of server to/from its peers
	};
	
	struct ConnectionsData
	{
		SocketPtr socket;
		std::queue<PacketPtr> message;
		std::condition_variable condVariable;
		std::mutex mutex;
		bool alive = true;
	};

	CEchoDistributedServer(const SConfig& config);
	~CEchoDistributedServer();

	void Run( const PeersList& peers);

private:
	void BroadCastMessage(PacketPtr& packet, const std::string& origin, 
		const std::string& sender);
	
	void ConnectToPeers(const PeersList& peers, SocketPtr& socket);
	void RemoveFromList(const SocketPtr& socket);
	void ProcessClient(SocketPtr& socket);

	bool AddClient(SocketPtr& socket, const std::string& id, bool active = false);
	bool ClientExist(const std::string& id);

	void ConfirmIdentity(SocketPtr& socket, bool peers);
	const std::string GetSocketId(const SocketPtr& socket);
	void RunSockets(SocketPtr& socket, bool peers);
	bool IsPeer(const std::string& id);
	PacketPtr ValidateIdentity(SocketPtr& ptr);
	bool BasicHandShake(const std::string& id);
	void CleanUpTransaction();

private:
	
	using FinishedTransactionMap = std::map<std::string, std::chrono::steady_clock::time_point>;
	FinishedTransactionMap m_finishedTransactions;
	std::mutex m_finishedMutex;

	CThreadPool m_pool;
	const SConfig& m_config;
	std::mutex m_mutex;
	std::mutex m_mutexCon;
	SocketMapId m_clients;
	std::atomic<int> m_msgId;

	using ConnectionDataPtr = std::shared_ptr<ConnectionsData>;
	std::vector<ConnectionDataPtr> m_connections;

};