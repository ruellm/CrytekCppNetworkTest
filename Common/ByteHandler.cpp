#include "ByteHandler.h"
#include <string>

namespace ByteHandler
{
	size_t ExtractSize_t(char* buffer)
	{
		size_t size = sizeof(size_t);
		char* input = new char[size];

		memcpy(input, buffer, sizeof(size_t));

		size_t result =  *((size_t*)input);
		delete[] input;

		return result;
	}
}