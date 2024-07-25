#pragma once

#include "external.hpp"

class logger {

	private:

		// stolen from Sequensa
		static std::string getTimestamp();
		static std::string getText(const std::string& type, std::stringstream& buffer);

		template <typename Arg, typename... Args>
		static void write(std::stringstream& buffer, Arg arg, Args... args) {
			buffer << std::forward<Arg>(arg);
    		((buffer << std::forward<Args>(args)), ...);
		}

		template <typename... Args>
		static void print(int level, const std::string& type, Args... args) {
			std::stringstream buffer;
			write(buffer, args...);

			std::cout << getText(type, buffer);
		}

	public:

		template <typename... Args>
		static void debug(Args... args) {
			print(0, "DEBUG", args...);
		}

		template <typename... Args>
		static void info(Args... args) {
			print(1, "INFO", args...);
		}

		template <typename... Args>
		static void warn(Args... args) {
			print(2, "WARN", args...);
		}

		template <typename... Args>
		static void error(Args... args) {
			print(3, "ERROR", args...);
		}

		template <typename... Args>
		static void fatal(Args... args) {
			print(4, "FATAL", args...);
		}

};
