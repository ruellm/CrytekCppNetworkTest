// Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "EchoDistributedServer.h"
#include "Config/ServerConfigLoader.h"
#include "Config/PeersConfigLoader.h"

#include <iostream>
#include <stdexcept>
#include <memory>

#ifdef WIN32
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#endif

struct SServerOptions
{
	std::string serverConfig;
	std::string peersConfig;
};

void VerifyServerConfig(const SServerConfig& config)
{
	if (config.id.size() == 0)
		throw std::runtime_error("Server id not found in config, must supply");
	else if (config.port == 0)
		throw std::runtime_error("Server port not found in config, must supply");
}

void VerifyPeersConfig(const PeersList& config)
{
	for (auto & c : config) {
		if (c.id.size() == 0)
			throw std::runtime_error("Peer id is empty");
		else if (c.id.size() == 0)
			throw std::runtime_error("Peer address is empty");
		else if (c.port == 0)
			throw std::runtime_error("Peer port is empty or set to 0");
	}
}

void Help()
{
	std::cout
		<< "Usage Server [OPTIONS]\n"
		<< "Options:\n\n"
		<< "    -h,--help  Display Help\n"
		<< "    --config <filename> loads the server config from file \n"
		<< "    --peers <filename> loads the peers server list from file \n";
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

		if (arg == "--config" && i + 1 < argc)
			options.serverConfig = argv[++i];
		else if (arg == "--peers" && i + 1 < argc)
			options.peersConfig = argv[++i];
		else if (arg == "-h" || arg == "--help")
			Help();
	}

	return options;
}

int main(int argc, char *argv[])
{
	auto options = LoadProgramOptions(argc, argv);

	if (options.serverConfig.size() == 0)
	{
		std::cout << "[ERROR] Server config not found or specified, "
			"run with -config [filename] " << std::endl;
		exit(1);
	}

	CServerConfigLoader sconfigLoader;
	if (!sconfigLoader.Load(options.serverConfig))
	{
		std::cout << "[ERROR] Unable to load server config file " 
			<< options.serverConfig << "\n";
		exit(1);
	}

	CPeersConfigLoader pconfigLoader;
	if (!pconfigLoader.Load(options.peersConfig))
	{
		std::cout << "[WARNING] Peers Config not found, running without peers " 
			<< options.peersConfig <<"\n";
	}

	auto config = sconfigLoader.Get();
	auto peers = pconfigLoader.Get();

	VerifyServerConfig(config);
	VerifyPeersConfig(peers);

	std::make_shared<CEchoDistributedServer>(config)->Run(peers);

	return 0;
}
