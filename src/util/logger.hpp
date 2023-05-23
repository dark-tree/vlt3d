#pragma once

#include "external.hpp"

class logger {

	private:

		// stolen from Sequensa
		static inline std::string getTimestamp(const char* pattern = "%H:%M:%S") {
			time_t rawtime = time(0);

			struct tm ts;
			char buf[255];

			ts = *gmtime(&rawtime);
			strftime(buf, sizeof(buf), pattern, &ts);

			return std::string(buf);
		}

		static inline std::string getText(const std::string& type, std::stringstream& buffer) {
			return getTimestamp() + " " + type + ": " + buffer.str() + "\n";
		}

		template<typename Arg, typename... Args>
		static void write(std::stringstream& buffer, Arg arg, Args... args) {
			buffer << std::forward<Arg>(arg);
    		((buffer << std::forward<Args>(args)), ...);
		}

		template<typename... Args>
		static void print(const std::string& type, Args... args) {
			std::stringstream buffer;
			write(buffer, args...);

			std::cout << getText(type, buffer);
		}

	public:

		template< typename... Args >
		static void info(Args... args) {
			print("INFO", args...);
		}

		template< typename... Args >
		static void warn(Args... args) {
			print("WARN", args...);
		}

		template< typename... Args >
		static void error(Args... args) {
			print("ERROR", args...);
		}

		template< typename... Args >
		static void fatal(Args... args) {
			print("FATAL", args...);
		}

};
