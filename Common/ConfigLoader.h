#pragma once

#include <string>

class CConfigLoader
{
public:
	bool Load(const std::string& file);

protected:
	virtual void Process(const std::string& line) = 0;
};