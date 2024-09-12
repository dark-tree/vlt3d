
#include "logger.hpp"

/*
 * Logger
 */

std::string Logger::getTimestamp() {
	auto now = std::chrono::system_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

	std::time_t epoch_time = std::chrono::system_clock::to_time_t(now);
	std::tm local_time = *std::localtime(&epoch_time);

	std::ostringstream oss;
	oss << std::put_time(&local_time, "%H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count();

	return oss.str();
}

std::string Logger::getText(const std::string& type, std::stringstream& buffer) {
	return getTimestamp() + " " + type + ": " + buffer.str() + "\n";
}

void Logger::setLevelMask(uint8_t value) {
	mask = value;
}

/*
 * logger
 */

Logger logger::global;

/*
 * LoggerLock
 */

LoggerLock::LoggerLock(Logger& logger, uint8_t mask)
: logger(logger), old(logger.mask) {
	logger.setLevelMask(mask);
}

LoggerLock::LoggerLock(uint8_t mask)
: LoggerLock(logger::global, mask) {}

LoggerLock::~LoggerLock() {
	logger.setLevelMask(old);
}