#include "EchoDistributedServer.h"
#include "Common/SocketFactory.h"
#include "Common/PacketBuilder.h"
#include "Common/PacketIdentity.h"
#include "Common/PacketSender.h"
#include "Common/Tokenizer.h"

#include <iostream>
#include <mutex>
#include <string.h>
#include <sstream>

#define MAX_BUFFER_LEN				1024
#define	FINISHED_TRANSACTION_SECS	5	// time finished transaction map store before deletion (in seconds)

CEchoDistributedServer::CEchoDistributedServer(const SConfig& config) 
	: m_config(config), m_msgId(0)
{}

CEchoDistributedServer::~CEchoDistributedServer()
{}

void CEchoDistributedServer::AddClient(SocketPtr& socket, const std::string& id)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_clients[id] = socket;
}

PacketPtr CEchoDistributedServer::ValidateIdentity(SocketPtr& socket)
{
	char buffer[MAX_BUFFER_LEN];

	int len = socket->Read(buffer, MAX_BUFFER_LEN);
	if (len <= 0)
	{
		std::cout << "[ERROR] Peer was not able to confirm this server identity \n";
		return nullptr;
	}

	auto packet = PacketBuilder::Build(buffer, len);
	if (packet->header.type != PacketType::Identity)
	{
		std::cout << "[ERROR] Expecting Identity response, recieved different \n";
		return nullptr;
	}

	// check if ID already exist
	SPacketIdentiy* derived = dynamic_cast<SPacketIdentiy*>(packet.get());
	auto iter = m_clients.find(derived->message);
	if (iter != m_clients.end())
	{
		std::cout << "[ERROR] ID Already exist denied loggin in " << derived->message << "\n";
		return nullptr;
	}

	// check white space on the ID
	if (derived->message.find(' ') != std::string::npos)
	{
		std::cout << "[ERROR] ID contains white space " << derived->message << "\n";
		return nullptr;
	}

	return std::move(packet);
}

void CEchoDistributedServer::ConnectToPeers(const PeersList& peers, SocketPtr& socket)
{
	m_pool.QueueTask([socket, this]() mutable 
	{
		RunSockets(socket, true);
	});

	m_pool.QueueTask([peers, this]() mutable 
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

			SPacketIdentiy identity(m_config.id);
			PacketSender::Send(&identity, socket);
			if (!ValidateIdentity(socket))
				return;

			AddClient(socket, p.id);

			m_pool.QueueTask([socket, this]() mutable 
			{
				ProcessClient(socket);
			});
		}
	});
}

bool CEchoDistributedServer::BasicHandShake(const std::string& id)
{
	return id.find("server_") == 0;
}

void CEchoDistributedServer::ConfirmIdentity(SocketPtr& socket, bool peers)
{
	std::cout << "[INFO] Confirming Identity ... ";
	m_pool.QueueTask([socket, peers, this]() mutable 
	{
		auto packet = ValidateIdentity(socket);
		if (!packet)
			return;

		SPacketIdentiy* derived = dynamic_cast<SPacketIdentiy*>(packet.get());

		// check for ID pattern
		if (peers && !BasicHandShake(derived->message))
		{
			std::cout << "[ERROR] Peers ID pattern is wrong not allowing connection \n";
			return;
		}

		AddClient(socket, derived->message);

		// Sends back the packet to confirm
		PacketSender::Send(packet.get(), socket);

		ProcessClient(socket);

		std::cout << "Confirmed Id " << derived->message << "\n";
	});
}

void CEchoDistributedServer::RunSockets(SocketPtr& socket, bool peers)
{
	while (true)
	{
		std::shared_ptr<ISocketBase> clientSocket = socket->Accept();
		if (!clientSocket)
			continue;

		std::cout << "[INFO] Client socket connected \n" << std::endl;
		ConfirmIdentity(clientSocket, peers);
	}
}

void CEchoDistributedServer::Run(const PeersList& peers)
{
	std::shared_ptr<ISocketBase> socket = SocketFactory::Create();
	if (!socket->CreateAsServer(m_config.port)) {
		std::cout << "[ERROR] Unable to Create Server at port " << m_config.port << "\n";
		return;
	}

	std::shared_ptr<ISocketBase> socketPeers = SocketFactory::Create();
	if (!socketPeers->CreateAsServer(m_config.port + 1)) {
		std::cout << "[ERROR] Unable to Create Server at port " << m_config.port + 1 << "\n";
		return;
	}

	m_pool.Initialize(m_config.numOfThreads, m_config.expand);

	std::cout
		<< "[INFO] Server (" << m_config.id << ") Running at port " << m_config.port
		<< " Listening to peers at port " << m_config.port + 1
		<< "\n\nWaiting for clients to connect... \n\n" << std::endl;

	ConnectToPeers(peers, socketPeers);

	RunSockets(socket, false);
}

void CEchoDistributedServer::RemoveFromList(const SocketPtr& socket)
{
	SocketMapId::iterator it = m_clients.begin();
	while (it != m_clients.end())
	{
		if (it->second == socket)
		{
			m_clients.erase(it->first);
			break;
		}

		it++;
	}
}

const std::string CEchoDistributedServer::GetSocketId(const SocketPtr& socket)
{
	SocketMapId::iterator it = m_clients.begin();
	while (it != m_clients.end())
	{
		if (it->second == socket)
			return it->first;

		it++;
	}

	return {};
}

void CEchoDistributedServer::CleanUpTransaction()
{
	std::cout << "[INFO] Running cleanup transaction cache \n";

	FinishedTransactionMap::iterator it = m_finishedTransactions.begin();
	while (it != m_finishedTransactions.end())
	{
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - it->second).count();

		if (duration > FINISHED_TRANSACTION_SECS)
			it = m_finishedTransactions.erase(it);
		else
			it++;
	}
}

void CEchoDistributedServer::BroadCastMessage(char* buffer, size_t len,
	const std::string& origin, const std::string& sender)
{
	SocketMapId::iterator it = m_clients.begin();

	Tokens tokens;
	Tokenizer::Tokenize(origin, " ", tokens);

	if (!IsPeer(tokens[0]))
	{
		if (m_finishedTransactions.find(origin) != m_finishedTransactions.end())
			return; //Data already processed in this server exiting;

		m_finishedTransactions[origin] = std::chrono::steady_clock::now();
		
		m_pool.QueueTask([this]() mutable
		{
			CleanUpTransaction();
		});
	}

	while (it != m_clients.end())
	{
		auto socket = it->second;
		m_pool.QueueTask([socket, buffer, len, origin, sender, tokens, this]() mutable 
		{
			// if receiver is a peer, set sender using new id?
			auto id = GetSocketId(socket);

			// Do not send packet to sender or to originator node
			// unless its a client node for echo
			if (id.compare(tokens[0]) == 0 ||
				(id.compare(sender) == 0 && IsPeer(sender)))
				return;

			if (!socket->Write((void*)buffer, (int)len))
			{
				RemoveFromList(socket);
			}
		});

		it++;
	}
}

bool CEchoDistributedServer::IsPeer(const std::string& id)
{
	return id.find("server_") == 0;
}

void CEchoDistributedServer::ProcessClient(SocketPtr& socket)
{
	char buffer[MAX_BUFFER_LEN];

	while (true)
	{
		if (!socket)
			return;

		std::cout << "[INFO] Waiting for Message \n" << std::endl;

		int len = socket->Read(buffer, MAX_BUFFER_LEN);

		// client connection is already closed or lost break the loop
		if (len <= 0)
		{
			RemoveFromList(socket);
			return;
		}

		std::cout << "[INFO] Broadcasting packet ... \n" << std::endl;

		// When a message arrived from a client, that means this is the first time it is being sent
		// mark the "from" address of the packet to the recieving node to avoid cyclc broadcast
		auto id = GetSocketId(socket);
		bool original = !IsPeer(id);
		std::string source;

		auto packet = PacketBuilder::Build(buffer, len);
		if (packet == nullptr)
			return;

		if (original)
		{
			std::stringstream ss;
			std::string transactionId;

			ss << m_msgId++;
			ss >> transactionId;

			// set originator node
			packet->from = m_config.id + " " + transactionId;
			size_t retSize = 0;
			auto buf = packet->Serialize(&retSize);

			memcpy(buffer, buf, retSize);
			len = (int)retSize;

			delete buf;
		}

		source = packet->from;

		BroadCastMessage(buffer, len, source, id);
	}

}
