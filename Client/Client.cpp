// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// Uncomment this line to run custom unit testing
//#define RUN_TEST

#include "Common/PacketBuilder.h"
#include "Common/PacketStringMessage.h"
#include "Common/SocketFactory.h"
#include "Config/ServerListLoader.h"
#include "Common/PacketIdentity.h"
#include "Common/PacketSender.h"
#include "Common/PacketReceiver.h"

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
#include <sstream>

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
	std::string id;

	int sendDelay = 5;
	int port = 0;
	int frequency = 1;

	int	reconnectRetry = 3;
	int reconnectDelay = 5;
	bool listenerOnly = false;
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

static std::string				g_id;

void LoginIdentity(const std::string& id);

void ExitWithError(const std::string& error)
{
	std::cout << error << "\n";
	exit(1);
}

void Help()
{
	std::cout
		<< "Usage Client [OPTIONS]\n"
		<< "Options:\n\n"
		<< "    -l,--listener run as listener (overrule other parameters; frequency, delay and message) \n"
		<< "    -h,--help  Display Help\n"
		<< "    -i, --id <string> Client identity in the network (REQUIRED). \n"
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
		<< "    to connect to the specified host first, if connection fails then it will go thru the list provided in the server list file. \n\n\n";

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

	std::cout << "[ERROR] Unable to connect to any server specified\n";
	exit(1);
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

bool OnDisconnect(const SClientOptions& options)
{
	// Pause this client, disabling writing in main thread
	g_pause = true;

	if (!Reconnect(options))
	{
		// force unpause waiting condition and end any waiting thread
		g_pause = false;
		g_done = true;

		std::cout << "Reconnect fail, Exiting ...\n" << std::endl;
		return false;
	}

	// re login
	LoginIdentity(g_id);

	// Reconnect happen unpause writing thread
	std::cout << "Connect success, Resuming operation ...\n" << std::endl;
	std::unique_lock<std::mutex> lock(g_mutex);
	g_condition.notify_one();
	g_pause = false;

	return true;
}

void ReadThread(const SClientOptions& options)
{
	while (true)
	{
		if (g_done)
			return;

		// poll until there is enough data in the socket
		PacketPtr packet = PacketReceiver::Receive(g_socket);

		// socket connection is lost
		if (packet == nullptr)
		{
			if(OnDisconnect(options))
				continue;  // continue thread by reading next data
		}

		ProcessPacket(std::move(packet));

		// give way for or thread to process
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
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
		else if (arg == "-i" || arg == "--id" && i + 1 < argc)
		{
			config.id = argv[++i];
		}
		else if (arg == "--reconnect-delay" && i + 1 < argc)
		{
			config.reconnectDelay = std::stoi(argv[++i]);
		}
		else if (arg == "-l" || arg == "--listener")
		{
			config.listenerOnly = true;
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

void LoginIdentity(const std::string& id)
{
	std::cout << "[INFO] Logging in client \n";

	if (id.size() == 0)
	{
		ExitWithError("[ERROR] Client ID is required ");
	}

	if (id.find(' ') != std::string::npos)
	{
		ExitWithError("[ERROR] ID contains white space " + id);
	}

	SPacketIdentiy identity(id);
	PacketSender::Send(&identity, g_socket);

	char buffer[MAX_BUFFER_LEN];
	
	int len = g_socket->Read(buffer, MAX_BUFFER_LEN);

	if (len <= 0)
		ExitWithError("[ERROR] Server Error in confirming Identity ");

	auto packet = PacketBuilder::Build(buffer, len);
	if (packet->header.type != PacketType::Identity)
		ExitWithError("[ERROR] Cant confirm Identity from server \n");

	SPacketIdentiy* derived = dynamic_cast<SPacketIdentiy*>(packet.get());
	if (derived->message != id)
		ExitWithError("[ERROR] Error in Identity Confirmation");

	// cache the ID to be used when reconnecting
	g_id = id;
}

void ValidateOptions(const SClientOptions& options)
{
	if (options.reconnectDelay == 0)
		ExitWithError("[ERROR] Reconnect Delay should not be 0");
}

void LaunchSenderThread(const SClientOptions& options)
{
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
		// or terminate application due to running out of retry attempt
		if (g_done)
			break;

		if (!options.listenerOnly)
		{
			std::stringstream ss;
			std::string msgID;

			ss << i++;
			ss >> msgID;

			std::string nmsg = options.message + " (" + msgID + ")";

			std::cout << "Sending message: " << nmsg << "\n" << std::endl;

			SPacketStringMessage packet;
			packet.message = nmsg;

			if (!PacketSender::Send(&packet, g_socket))
			{
				OnDisconnect(options);
				continue;
			}

			if (options.frequency != -1)
			{
				if (i >= options.frequency)
					break;
			}
		}

		std::this_thread::sleep_for(std::chrono::seconds(options.sendDelay));

		if (options.sendDelay == 0)
		{
			// Add a small delay to prevent CPU hog and endless spinning and provide CPU time to the other (read)
			// thread when send delay is 0 (no delay)
			// this can be commented out for faster systems to speed up and rapidly send message
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
	}
}

int MainClient(int argc, char *argv[])
{
	std::cout << "===== Distributed Echo Client v2.0 ==== \n\n";

	// prepare the options
	auto options = LoadProgramOptions(argc, argv);

	// validate options
	ValidateOptions(options);

	// build the server list to connect to
	BuildServerList(options);

	// attempt to connect from server list
	AttemptConnect();

	// Announce our identity to the server
	LoginIdentity(options.id);

	std::cout << " Running Client ID " << options.id 
		<< (options.listenerOnly? " as LISTENER " : "") <<" \n";

	// Since client receives information from server from broadcast from other client/servers
	// a read thread is launched separately
	std::thread thread(ReadThread, options);

	LaunchSenderThread(options);

	// kill all thread when frequency is consumed
	g_done = true;

	//// this client is done set flag to compelte the read thread
	g_pause = false;

	// Wait for the thread to finish; we stay here until it is done
	thread.join();

	// cleanup whatever initialized (for windows socket)
	SocketFactory::Destroy();

	// add interactive mode before exit to notify user processing is done
	if (options.frequency != -1)
	{
		std::cout << "Processing complete, Press Return key to exit... \n";
		getchar();
	}

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
