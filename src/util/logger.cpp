
#include "logger.hpp"

std::string logger::getTimestamp() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	time_t epoch = tv.tv_sec;

	char clock_string[128];
	struct tm ts = *localtime(&epoch);
	strftime(clock_string, sizeof clock_string, "%H:%M:%S", &ts);

	char milli_string[255];
	snprintf(milli_string, sizeof milli_string, "%s.%03ld", clock_string, (tv.tv_usec / 1000));

	return std::string(milli_string);
}

std::string logger::getText(const std::string& type, std::stringstream& buffer) {
	return getTimestamp() + " " + type + ": " + buffer.str() + "\n";
}