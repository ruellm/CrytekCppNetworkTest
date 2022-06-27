#pragma once

#include "PeersConfig.h"
#include "Common/ConfigLoader.h"

#include <vector>

class CPeersConfigLoader : public CConfigLoader
{
public:
	CPeersConfigLoader() = default;

	virtual void Process(const std::string& line) override;
	const PeersList& Get();

private:
	PeersList m_peers;
};
