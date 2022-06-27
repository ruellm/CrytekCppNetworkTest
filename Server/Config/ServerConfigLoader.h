#pragma once

#include "ServerConfig.h"
#include "Common/ConfigLoader.h"

class CServerConfigLoader : public CConfigLoader
{
public:
	CServerConfigLoader() = default;

	virtual void Process(const std::string& line) override;
	const SServerConfig& Get();
private:
	SServerConfig m_config;
};
