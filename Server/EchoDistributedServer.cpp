#include "EchoDistributedServer.h"
#include "SocketFactory.h"

#include <iostream>
#include <mutex>

CEchoDistributedServer::CEchoDistributedServer(const SServerConfig& config) : m_config(config)
{}

CEchoDistributedServer::~CEchoDistributedServer()
{}

void CEchoDistributedServer::AddSocket(SocketPtr& socket)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_clientList.push_back(socket);
}

void CEchoDistributedServer::ConnectToPeers(const PeersList& peers)
{
	for (auto p : peers)
	{
		if (p.id == m_config.id)
			continue;

		std::cout << "[INFO] Attempt Connect to Peer " << p.address << " port " << p.port << "...";
		std::shared_ptr<ISocketBase> socket = SocketFactory::Create();
		if (!socket->Connect(p.address.c_str(), p.port)) {
			std::cout << " Error\n";
			continue;
		}

		std::cout << "Success!\n";
		AddSocket(socket);

		m_pool.QueueTask([socket, this]() mutable {
			ProcessClient(socket, true);
		});
	}
}

void CEchoDistributedServer::Run(const PeersList& peers)
{
	std::shared_ptr<ISocketBase> socket = SocketFactory::Create();
	if (!socket->CreateAsServer(m_config.port)) {
		std::cout << "[ERROR] Unable to Create Server at port " << std::endl;
		return;
	}

	m_pool.Initialize(m_config.numOfThreads);

	ConnectToPeers(peers);

	std::cout << "[INFO] Server Running at port " << m_config.port 
		<< "\nWaiting for clients to connect... " << std::endl;
	
	while (true)
	{
		std::shared_ptr<ISocketBase> clientSocket = socket->Accept();
		if (!clientSocket)
			continue;

		std::cout << "[INFO] Client socket connected \n" << std::endl;

		AddSocket(clientSocket);

		m_pool.QueueTask([clientSocket, this]() mutable {
			ProcessClient(clientSocket);
		});
	}
}

void CEchoDistributedServer::RemoveFromList(const SocketPtr& socket)
{
	SocketList::iterator it = m_clientList.begin();
	while (it != m_clientList.end())
	{
		if (*it == socket)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			it = m_clientList.erase(it);
		}
		else {
			it++;
		}
	}
}

void CEchoDistributedServer::BroadCastMessage(char* buffer, int len, const SocketPtr& from, bool peer)
{
	for (auto socket : m_clientList)
	{
		// if message is from peer, do not rebroadcast
		if (peer && from == socket)
			continue;

		m_pool.QueueTask([socket, buffer, len, this]() {
			if (!socket->Write((void*)buffer, len))
			{
				RemoveFromList(socket);
			}
		});
	}
}

void CEchoDistributedServer::ProcessClient(SocketPtr& socket, bool peer)
{
	const int MAX_BUFFER_LEN = 2000;
	char buffer[MAX_BUFFER_LEN];

	while (true)
	{
		if (!socket)
			return;

		std::cout << "[INFO] Waiting for read \n" << std::endl;

		int len = socket->Read(buffer, MAX_BUFFER_LEN);

		// client connection is already closed or lost break the loop
		if (len <= 0)
		{
			RemoveFromList(socket);
			return;
		}

		std::cout << "[INFO]  Broadcasting message \n" << std::endl;
		BroadCastMessage(buffer, len, socket, peer);
	}
}