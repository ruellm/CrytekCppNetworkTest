#pragma once

#include <string>

struct SServerConfig
{
	std::string id;		// server identity
	int port;			// server port
	int numOfThreads;	// the number of threads in the thread pool
	bool expand;		// will the server expand and create more thread if it runs out
};