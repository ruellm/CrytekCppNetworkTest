#include "PeersConfigLoader.h"
#include "Common/Tokenizer.h"

void CPeersConfigLoader::Process(const std::string& line)
{
	Tokens tokens;
	Tokenizer::Tokenize(line, " ", tokens);

	if (tokens.size() != 3)
		return;

	SPeersConfig config;
	config.id = tokens[0];
	config.address = tokens[1];
	config.port = std::stoi(tokens[2]);

	m_peers.push_back(config);
}

const PeersList& CPeersConfigLoader::Get()
{
	return m_peers;
}
