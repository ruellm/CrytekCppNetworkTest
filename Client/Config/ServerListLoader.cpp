#include "ServerListLoader.h"
#include "Common/Tokenizer.h"

void CServerListLoader::Process(const std::string& line)
{
	Tokens tokens;
	Tokenizer::Tokenize(line, " ", tokens);

	if (tokens.size() != 2)
		return;

	SServerIdentity config;
	config.host = tokens[0];
	config.port = std::stoi(tokens[1]);

	m_servers.push_back(config);
}

const ServerList& CServerListLoader::Get()
{
	return m_servers;
}
