#pragma once

#include <string>
#include <vector>

using Tokens = std::vector<std::string>;

class CConfigLoader
{
public:
	bool Load(const std::string& file);

protected:
	virtual void Process(const std::string& line) = 0;
	void Tokenize(const std::string& line, const std::string& delimiters, Tokens& tokens);
};