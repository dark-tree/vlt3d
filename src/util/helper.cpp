
#include "helper.hpp"

void split(std::vector<std::string>& vec, const std::string &s, char delim) {
	std::vector<std::string> result;
	std::stringstream ss(s);
	std::string item;

	while( getline(ss, item, delim) ) {
		if( item.size() > 0 ) {
			vec.push_back(item);
		}
	}
}

