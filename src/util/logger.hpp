#pragma once

#include "external.hpp"

template <typename T>
std::ostream& operator<< (std::ostream& out, const glm::vec<2, T>& vec) {
	return out << vec[0] << " " << vec[1];
}

template <typename T>
std::ostream& operator<< (std::ostream& out, const glm::vec<3, T>& vec) {
	return out << glm::vec2(vec) << " " << vec[2];
}

template <typename T>
std::ostream& operator<< (std::ostream& out, const glm::vec<4, T>& vec) {
	return out << glm::vec3(vec) << " " << vec[3];
}

class Logger {

	public:

		enum Level : uint8_t {
			SILENT  = 0b00000,
			DEBUG   = 0b00001,
			INFO    = 0b00010,
			WARN    = 0b00100,
			ERROR   = 0b01000,
			FATAL   = 0b10000,
			VERBOSE = 0b11111
		};

	private:

		uint8_t mask = VERBOSE;

		// stolen from Sequensa
		std::string getTimestamp();
		std::string getText(const std::string& type, std::stringstream& buffer);

		template <typename Arg, typename... Args>
		void write(std::stringstream& buffer, Arg arg, Args... args) {
			buffer << std::forward<Arg>(arg);
			((buffer << std::forward<Args>(args)), ...);
		}

		template <typename... Args>
		void print(Level level, const std::string& type, Args... args) {
			if (level & mask) {
				std::stringstream buffer;
				write(buffer, args...);

				std::cout << getText(type, buffer);
			}
		}

		friend class LoggerLock;

	public:

		/**
		 * Specifies which log statements should actually actually produce output
		 */
		void setLevelMask(uint8_t value);

	public:

		template <typename... Args>
		void debug(Args... args) {
			#if !defined(NDEBUG)
			print(Level::DEBUG, "DEBUG", args...);
			#endif
		}

		template <typename... Args>
		void info(Args... args) {
			print(Level::INFO, "INFO", args...);
		}

		template <typename... Args>
		void warn(Args... args) {
			print(Level::WARN, "WARN", args...);
		}

		template <typename... Args>
		void error(Args... args) {
			print(Level::ERROR, "ERROR", args...);
		}

		template <typename... Args>
		void fatal(Args... args) {
			print(Level::FATAL, "FATAL", args...);
		}

};

namespace logger {

	extern Logger global;

	template <typename... Args>
	static void debug(Args... args) {
		global.debug(args...);
	}

	template <typename... Args>
	static void info(Args... args) {
		global.info(args...);
	}

	template <typename... Args>
	static void warn(Args... args) {
		global.warn(args...);
	}

	template <typename... Args>
	static void error(Args... args) {
		global.error(args...);
	}

	template <typename... Args>
	static void fatal(Args... args) {
		global.fatal(args...);
	}

}

/**
 * RAII wrapper around logger mode setting
 */
class LoggerLock {

	private:

		Logger& logger;
		const uint8_t old;

	public:

		LoggerLock(Logger& logger, uint8_t mask);
		LoggerLock(uint8_t mask);

		~LoggerLock();

};