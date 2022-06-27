#include "ServerConfigLoader.h"

void CServerConfigLoader::Process(const std::string& line)
{
	Tokens tokens;
	Tokenize(line, " ", tokens);
	
	if (tokens.size() != 2)
		return;

	else if (tokens[0].compare("id") == 0)
		m_config.id = tokens[1];
	else if (tokens[0].compare("port") == 0)
		m_config.port = std::stoi(tokens[1]);
	else if (tokens[0].compare("numOfThreads") == 0)
		m_config.numOfThreads = std::stoi(tokens[1]);
}

const SServerConfig& CServerConfigLoader::Get()
{
	return m_config;
}