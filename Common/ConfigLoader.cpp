#include "ConfigLoader.h"
#include <fstream>

bool CConfigLoader::Load(const std::string& configFile)
{
	std::ifstream file(configFile.c_str());
	std::string line;

	if (file.fail())
		return false;

	while (std::getline(file, line)) {

		if (!line.length()) continue;

		if (line[0] == '#') continue;
		if (line[0] == ';') continue;

		Process(line);

	}
	return true;
}
