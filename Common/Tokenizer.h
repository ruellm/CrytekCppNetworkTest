#pragma once

#include <string>
#include <vector>

using Tokens = std::vector<std::string>;

namespace Tokenizer
{
	void Tokenize(const std::string& line, const std::string& delimiters, Tokens& tokens);
}