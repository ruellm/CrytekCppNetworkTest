#include "Tokenizer.h"

namespace Tokenizer
{
	void Tokenize(const std::string& str, const std::string& delimiters, Tokens& tokens)
	{
		using namespace std;

		// Skip delimiters at beginning.
		string::size_type lastPos = str.find_first_not_of(delimiters, 0);
		// Find first "non-delimiter".
		string::size_type pos = str.find_first_of(delimiters, lastPos);

		if (string::npos == pos) 
		{
			//no delimeter found insert the string and return
			tokens.push_back(str.substr(0, str.length()));
			return;
		}

		while (string::npos != pos || string::npos != lastPos)
		{
			// Found a token, add it to the vector.    
			tokens.push_back(str.substr(lastPos, pos - lastPos));

			// Skip delimiters.  Note the "not_of"
			lastPos = str.find_first_not_of(delimiters, pos);
			// Find next "non-delimiter"
			pos = str.find_first_of(delimiters, lastPos);
		}
	}

}