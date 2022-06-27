// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// Uncomment this line to run custom unit testing
//#define RUN_TEST

#include "Network/PacketBuilder.h"
#include "Common/SocketFactory.h"
#include "ServerListLoader.h"

#ifdef RUN_TEST
#include "Test/Test.h"
#endif

#include <iostream>
#include <string>
#include <stdexcept>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>

#ifdef WIN32
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#endif

// Options for client operations
struct SClientOptions
{
	std::string host;
	std::string serverFile;
	std::string message = "Hello World";

	int sendDelay = 5;
	int port = 0;
	int frequency = 1;

	int	reconnectRetry = 3;
	int reconnectDelay = 5;
};

// Global definitions for client
static SocketPtr				g_socket = nullptr;
static std::string				g_host;
static int						g_port = 0;
static int						g_serverListIndex = 0;
static ServerList				g_servers;

static std::atomic<bool>		g_done(false);
static std::atomic<bool>		g_pause(false);

static std::condition_variable	g_condition;
static std::mutex				g_mutex;
static std::mutex				g_socketMutex;

void Send(IPacketBase* packet)
{
	char* buffer = nullptr;
	size_t len = 0;
	buffer = packet->Serialize(&len);

	// write the packet
	int res = g_socket->Write(buffer, (int)len);
	if (res <= 0)
	{
		std::cout << "Unable to send message \n" << std::endl;
	}

	delete[] buffer;
}

void SendStringMessage(const std::string& message)
{
	SPacketStringMessage packet;
	packet.Set(message);

	Send(&packet);
}

void Help()
{
	std::cout
		<< "Usage Client [OPTIONS]\n"
		<< "Options:\n\n"
		<< "    -h,--help  Display Help\n"
		<< "    --frequency <number> number of times this client will send the message, (-1) sends forever, (default is 1). \n"
		<< "    --reconnect-retry <number> number of times to retry reconnect when disconnected from server (default is 3). \n"
		<< "    --reconnect-delay <number> number of seconds to wait before next reconnect attempt in seconds (default is 5 seconds). \n"
		<< "    --message <string> Set the Message to sent to the server (default is 'Hello World'). \n"
		<< "    --delay <integer> delay in sending message in seconds (default is 5 seconds). \n"
		<< "    --host <address> Ip address or host name of the server to connect to \n"
		<< "    --port <number> port of the host name of the server to connect to \n"
		<< "    --servers <filename> specify the file that contains the list of servers to connect to. The client connects \n"
		<< "      to the first address in the list, if connection fails then it will move to the next one. \n\n"
		<< "General Guideline:\n\n"
		<< "    When --host and --port is specified, it takes higher precedence than --servers. That means the client will attempt\n"
		<< "    to connect to the specified host first, if connection fails then it will go thru the list provided in the server list file. \n\n";

	exit(0);
}

void AttemptConnect()
{
	g_socket = SocketFactory::Create();

	for (auto server : g_servers)
	{
		std::cout << "Attempting connect to " << server.host << " port " << server.port << std::endl;

		if (g_socket->Connect(server.host, server.port)) {
			std::cout << "... Connected! \n" << std::endl;
			g_host = server.host;
			g_port = server.port;
			return;
		}
	}

	throw std::runtime_error("Unable to connect to any server specified");
}

void ProcessPacket(PacketPtr packet)
{
	switch (packet->header.type)
	{
	case PacketType::StringMessage:
		{
			SPacketStringMessage* derived = dynamic_cast<SPacketStringMessage*>(packet.get());
			std::cout << "String message received: " << derived->message << " \n" << std::endl;
			break;
		}
	}
}

bool Reconnect(const SClientOptions& options)
{
	for (int i = 0; i < options.reconnectRetry; i++)
	{
		std::cout << "Attepting (#" << i << ") connect to " << g_host
			<< " port " << g_port << "...\n";

		if (g_socket->Connect(g_host, g_port))
			return true;

		std::this_thread::sleep_for(std::chrono::seconds(options.reconnectDelay));
	}

	return false;
}

void ReadThread(const SClientOptions& options)
{
	const int MAX_BUFFER_LEN = 2000;
	char buffer[MAX_BUFFER_LEN];

	while (true)
	{
		int len = g_socket->Read(buffer, MAX_BUFFER_LEN);
	
		if (g_done)
			return;

		// socket connection is lost
		if (len <= 0)
		{
			// Pause this client, disabling writing in main thread
			g_pause = true;

			if (!Reconnect(options))
			{
				// force unpause waiting condition and end any waiting thread
				g_pause = false;
				g_done = true;

				std::cout << "Reconnect fail, Exiting ...\n" << std::endl;
				return;
			}

			// Reconnect happen unpause writing thread
			std::cout << "Connect success, Resuming operation ...\n" << std::endl;
			std::unique_lock<std::mutex> lock(g_mutex);
			g_condition.notify_one();
			g_pause = false;

			continue;  // continue thread by reading next data
		}

		auto packet = PacketBuilder::Build(buffer, len);
		ProcessPacket(std::move(packet));
	}
}


SClientOptions LoadProgramOptions(int argc, char *argv[])
{
	SClientOptions config;
	if (argc == 1)
		Help();

	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];

		if (arg == "-h" || arg == "--help")
			Help();
		else if (arg == "--frequency" && i + 1 < argc)
		{
			config.frequency = std::stoi(argv[++i]);
		}
		else if (arg == "--host" && i + 1 < argc)
		{
			config.host = argv[++i];
		}
		else if (arg == "--port" && i + 1 < argc)
		{
			config.port = std::stoi(argv[++i]);
		}
		else if (arg == "--servers" && i + 1 < argc)
		{
			config.serverFile = argv[++i];
		}
		else if (arg == "--message" && i + 1 < argc)
		{
			config.message = argv[++i];
		}
		else if (arg == "--reconnect-retry" && i + 1 < argc)
		{
			config.reconnectRetry = std::stoi(argv[++i]);
		}
		else if (arg == "--delay" && i + 1 < argc)
		{
			config.sendDelay = std::stoi(argv[++i]);
		}
		else if (arg == "--reconnect-delay" && i + 1 < argc)
		{
			config.reconnectDelay = std::stoi(argv[++i]);
		}
	}

	return config;
}

void BuildServerList(const SClientOptions& options)
{
	if ((options.host.size() == 0 || options.port == 0) && options.serverFile.size() == 0)
	{
		std::cout << "[ERROR] Server to connect not provided correctly,"
			"no Host/Port or server list provided. \n" << std::endl;

		exit(1);
	}
	else
	{
		if (options.host.size() != 0 && options.port != 0)
		{
			SServerIdentity s;
			s.host = options.host;
			s.port = options.port;
			g_servers.push_back(s);
		}
	}

	if (options.serverFile.size() > 0)
	{
		CServerListLoader loader;
		if (!loader.Load(options.serverFile))
		{
			std::cout << "[WARNING] Unable to load server list file "
				<< options.serverFile << " \n" << std::endl;
		}
		else
		{
			auto s = loader.Get();
			g_servers.insert(g_servers.end(), s.begin(), s.end());
		}
	}
}

int MainClient(int argc, char *argv[])
{
	// prepare the options
	auto options = LoadProgramOptions(argc, argv);

	// build the server list to connect to
	BuildServerList(options);

	// attempt to connect from server list
	AttemptConnect();

	// Since client receives information from server from broadcast from other client/servers
	// a read thread is launched separately
	std::thread thread(ReadThread, options);

	// Start Sending Message (in main thread)
	int i = 0;
	while (true)
	{
		if (g_pause)
		{
			std::cout << "Paused sending ...\n" << std::endl;
			// When disconnected from server, wait for it to resume
			std::unique_lock<std::mutex> lock(g_mutex);
			g_condition.wait(lock, [] {
				return g_pause == false;
			});
		}

		// Check if we are still allowed to continue after pause 
		// or terminate application due to connection problem from server
		if (g_done)
			break;

		std::cout << "Sending (" << i << ") with message: " << options.message << "\n" << std::endl;
		SendStringMessage(options.message);

		if (options.frequency != -1)
		{
			if (i < options.frequency)
				i++;
			else
				break;
		}

		std::this_thread::sleep_for(std::chrono::seconds(options.sendDelay));
	}

	// kill all thread when frequency is consumed
	g_done = true;

	//// this client is done set flag to compelte the read thread
	g_pause = false;

	// Wait for the thread to finish; we stay here until it is done
	thread.join();
	
	// cleanup whatever initialized (for windows socket)
	SocketFactory::Destroy();

	return 0;
}

#ifdef RUN_TEST
int RunTest(int argc, char *argv[])
{
	Test::TestSerialize();
	return 0;
}
#endif

int main(int argc, char *argv[])
{
#ifdef RUN_TEST
	return  RunTest(argc, argv);
#else
	return MainClient(argc, argv);
#endif
}
