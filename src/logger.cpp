
#include "logger.hpp"

// stolen from Sequensa
std::string logger::getTimestamp( const char* pattern ) {
	time_t rawtime = time(0); 

	struct tm ts;
	char buf[255];

	ts = *gmtime(&rawtime);
	strftime(buf, sizeof(buf), pattern, &ts);

	return std::string(buf);
}

std::string logger::getText( const std::string& type, std::stringstream& buffer ) {
	return getTimestamp() + " " + type + ": " + buffer.str() + "\n";
}

