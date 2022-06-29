// Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "EchoDistributedServer.h"
#include "Config/PeersConfigLoader.h"

#include <iostream>
#include <stdexcept>
#include <memory>
#include <assert.h>
#include <string.h>

#ifdef WIN32
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#endif

struct SServerOptions
{
	CEchoDistributedServer::SConfig config;
	std::string peersConfig;
};

void ExitWithError(const std::string& error)
{
	std::cout << error << "\n";
	exit(1);
}

void VerifyServerConfig(const CEchoDistributedServer::SConfig& config)
{
	if (config.port == 0) 
		ExitWithError("[ERROR] Port is not specified");

	if (config.id.size() == 0) 
		ExitWithError("[ERROR] Server ID not found");
}

void VerifyPeersConfig(const PeersList& config)
{
	for (auto & c : config) {
		if (c.id.size() == 0)
			ExitWithError("Peer id is empty");
		else if (c.id.size() == 0)
			ExitWithError("Peer address is empty");
		else if (c.port == 0)
			ExitWithError("Peer port is empty or set to 0");
	}
}

void Help()
{
	std::cout
		<< "Usage Server [OPTIONS]\n"
		<< "Options:\n\n"
		<< "    -h,--help  Display Help\n"
		<< "    -i,--id <string> server identity (REQUIRED) \n"
		<< "    -p,--port <number> server port to listens to (REQUIRED) \n"
		<< "    --thread-count <number> max number of threads in the thread pool to (default 100)\n"
		<< "    --expand <true/false> will the server expand and create more thread if it runs out (default false) \n"
		<< "    --peers <filename> loads the peers server list from file (optional) \n\n\n";
	
	exit(1);
}

SServerOptions LoadProgramOptions(int argc, char *argv[])
{
	SServerOptions options;

	if (argc == 1)
		Help();

	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg == "-p" || arg == "--port" && i + 1 < argc)
			options.config.port = atoi(argv[++i]);
		else if (arg == "--thread-count" && i + 1 < argc)
			options.config.numOfThreads = atoi(argv[++i]);
		else if (arg == "--expand" && i + 1 < argc)
			options.config.expand = strcmp(argv[++i], "true") == 0 ? true : false;
		else if (arg == "-i" || arg == "--id" && i + 1 < argc)
			options.config.id = argv[++i];
		else if (arg == "--peers" && i + 1 < argc)
			options.peersConfig = argv[++i];
		else if (arg == "-h" || arg == "--help")
			Help();
	}

	return options;
}

int main(int argc, char *argv[])
{
	std::cout << "===== Distributed Echo Server v1.0 ==== \n\n";

	auto options = LoadProgramOptions(argc, argv);

	CPeersConfigLoader pconfigLoader;
	if (!pconfigLoader.Load(options.peersConfig))
	{
		std::cout << "[WARNING] Peers Config not found, running without peers " 
			<< options.peersConfig <<"\n";
	}

	auto peers = pconfigLoader.Get();

	VerifyServerConfig(options.config);
	VerifyPeersConfig(peers);

	std::make_shared<CEchoDistributedServer>(options.config)->Run(peers);

	return 0;
}
