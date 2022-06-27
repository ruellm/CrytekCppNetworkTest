#pragma once

#include <string>
#include <vector>

struct SPeersConfig
{
	std::string id;
	std::string address;
	int port;
};

using PeersList = std::vector<SPeersConfig>;