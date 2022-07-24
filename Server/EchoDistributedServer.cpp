#include "EchoDistributedServer.h"
#include "Common/SocketFactory.h"
#include "Common/PacketBuilder.h"
#include "Common/PacketIdentity.h"
#include "Common/PacketSender.h"
#include "Common/Tokenizer.h"
#include "Common/PacketReceiver.h"
#include "Common/PacketSender.h"

#include <iostream>
#include <mutex>
#include <string.h>
#include <sstream>

// time finished transaction map store before deletion (in seconds)
#define	FINISHED_TRANSACTION_SECS	60	

CEchoDistributedServer::CEchoDistributedServer(const SConfig& config) 
	: m_config(config), m_msgId(0)
{}

CEchoDistributedServer::~CEchoDistributedServer()
{}

bool CEchoDistributedServer::ClientExist(const std::string& id)
{
	auto iter = m_clients.find(id);
	return (iter != m_clients.end());
}

bool CEchoDistributedServer::AddClient(SocketPtr& socket, const std::string& id, bool active)
{
	std::string connectionType = active ? "Active" : "Listener";

	if (ClientExist(id))
	{
		std::cout << "[WARNING] connection to " << id << " as " << connectionType
			<< " previously established, dropping connection \n";

		return false;
	}

	std::lock_guard<std::mutex> lock(m_mutex);
	std::cout << "[INFO] connection to " << id << " established as "<< connectionType <<"! \n";
	m_clients[id] = socket;
	return true;
}

PacketPtr CEchoDistributedServer::ValidateIdentity(SocketPtr& socket)
{
	PacketPtr packet = nullptr;
	if(PacketReceiver::Wait(socket))
		packet = PacketReceiver::Receive(socket);
	
	if (packet == nullptr)
	{
		std::cout << "[ERROR] Peer Rejected connection (validation or connection is already active)... \n";
		return nullptr;
	}

	if (packet->header.type != PacketType::Identity)
	{
		std::cout << "[ERROR] Expecting Identity response, recieved different \n";
		return nullptr;
	}

	// check if ID already exist
	SPacketIdentiy* derived = dynamic_cast<SPacketIdentiy*>(packet.get());
	if (ClientExist(derived->message))
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
	// Run and listen from peers socket
	m_pool.QueueTask([socket, this]() mutable 
	{
		RunSockets(socket, true);
	});

	// connects to each peers
	m_pool.QueueTask([peers, this]() mutable 
	{
		for (auto p : peers)
		{
			if (p.id == m_config.id)
				continue;

			std::cout << "[INFO] Attempt Connect to Peer " << p.address << " port " << p.port << "("<< p.id << ")...";
			SocketPtr socket = SocketFactory::Create();
			if (!socket->Connect(p.address.c_str(), p.port)) {
				std::cout << " Error\n";
				continue;
			}

			SPacketIdentiy identity(m_config.id);
			PacketSender::Send(&identity, socket);
			if (!ValidateIdentity(socket))
				continue;

			if (!AddClient(socket, p.id, true))
				continue;

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
	std::cout << "[INFO] Confirming Identity ... \n";
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

		if (!AddClient(socket, derived->message))
			return;

		// Sends back the packet to confirm
		PacketSender::Send(packet.get(), socket);

		ProcessClient(socket);

		std::cout << "[INFO] Confirmed Id " << derived->message << "\n";
	});
}

void CEchoDistributedServer::RunSockets(SocketPtr& socket, bool peers)
{
	while (true)
	{
		SocketPtr clientSocket = socket->Accept();
		if (!clientSocket)
			continue;

		// make it as an unblocking socket
		clientSocket->UnBlock();

		std::cout << "[INFO] Client socket connected \n" << std::endl;
		ConfirmIdentity(clientSocket, peers);
	}
}

void CEchoDistributedServer::Run(const PeersList& peers)
{
	SocketPtr socket = SocketFactory::Create();
	if (!socket->CreateAsServer(m_config.port)) {
		std::cout << "[ERROR] Unable to Create Server at port " << m_config.port << "\n";
		return;
	}

	SocketPtr socketPeers = SocketFactory::Create();
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
	{
		std::lock_guard<std::mutex> lock(m_mutex);
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

	{
		std::lock_guard<std::mutex> lock(m_mutexCon);
		std::vector<ConnectionDataPtr>::iterator itc = m_connections.begin();
		while (itc != m_connections.end())
		{
			if ((*itc)->socket == socket)
			{
				(*itc)->alive = false;

				m_connections.erase(itc);
				break;
			}

			itc++;
		}
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
	std::lock_guard<std::mutex> lock(m_finishedMutex);

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

void CEchoDistributedServer::BroadCastMessage(PacketPtr& packet, 
	const std::string& origin, const std::string& sender)
{
	Tokens tokens;
	Tokenizer::Tokenize(origin, " ", tokens);

	if (!IsPeer(tokens[0]))
	{
		if (m_finishedTransactions.find(origin) != m_finishedTransactions.end())
			return; //Data already processed in this server exiting;

		{
			std::lock_guard<std::mutex> lock(m_finishedMutex);
			m_finishedTransactions[origin] = std::chrono::steady_clock::now();
		}

		m_pool.QueueTask([this]() mutable
		{
			CleanUpTransaction();
		});
	}

	// copy to temporary to prevent timing issue when the list changes
	SocketMapId temp;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		temp.insert(m_clients.begin(), m_clients.end());
	}

	SocketMapId::iterator it = temp.begin();
	while (it != temp.end())
	{
		auto socket = it->second;
		m_pool.QueueTask([socket, packet, origin, sender, tokens, this]() mutable 
		{
			// if receiver is a peer, set sender using new id?
			auto id = GetSocketId(socket);

			// Do not send packet to sender or to originator node
			// unless its a client node for echo
			if (id.compare(tokens[0]) == 0 ||
				(id.compare(sender) == 0 && IsPeer(sender)))
				return;

			if (!PacketSender::Wait(socket) ||
				!PacketSender::Send(packet.get(), socket))
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
	auto connection = std::make_shared<ConnectionsData>();
	connection->socket = socket;

	{
		std::lock_guard<std::mutex> lock(m_mutexCon);
		m_connections.push_back(connection);
	}

	// launch read thread for this socket
	m_pool.QueueTask([socket, connection, this]() mutable
	{
		while (true)
		{
			if (!socket)
				return;

			std::cout << "[INFO] Waiting for Message \n" << std::endl;

			PacketPtr packet = nullptr;
			if (PacketReceiver::Wait(socket))
				packet = PacketReceiver::Receive(socket);

			if (packet == nullptr)
			{
				RemoveFromList(socket);
				return;
			}
	
			{
				std::unique_lock<std::mutex> lock(connection->mutex);
				connection->message.push(packet);
			}

			connection->condVariable.notify_one();
		}
	});

	// launch write thread for this socket
	m_pool.QueueTask([socket, connection, this]() mutable
	{
		while (connection->alive)
		{
			if (!socket)
				return;

			PacketPtr packet = nullptr;
			{
				std::unique_lock<std::mutex> lock(connection->mutex);
				connection->condVariable.wait(lock, [connection, this] {
					return !connection->message.empty();
				});

				if (!connection->message.empty())
				{
					packet = connection->message.front();
					connection->message.pop();
				}
			}
			
			if (packet == nullptr)
				continue;

			// When a message arrived from a client, that means this is the first time it is being sent
			// mark the "from" address of the packet to the recieving node to avoid cyclc broadcast
			auto id = GetSocketId(socket);
			bool original = !IsPeer(id);
			std::string source;

			if (original)
			{
				std::stringstream ss;
				std::string transactionId;

				ss << m_msgId++;
				ss >> transactionId;

				// set originator node and reserialize
				packet->from = m_config.id + " " + transactionId;
			}

			std::cout << "[INFO] Broadcasting packet ... \n" << std::endl;

			source = packet->from;
			BroadCastMessage(packet, source, id);
		}
	});
}
