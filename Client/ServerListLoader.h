#pragma once

#include "ConfigLoader.h"

#include <vector>
#include <string>

struct SServerIdentity
{
	std::string host;
	int port;
};

using ServerList = std::vector<SServerIdentity>;

class CServerListLoader : public CConfigLoader
{
public:
	CServerListLoader() = default;

	virtual void Process(const std::string& line) override;
	const ServerList& Get();

private:
	ServerList m_servers;
};
