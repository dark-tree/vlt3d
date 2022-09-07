#pragma once

#include <include/external.hpp>
#include <util/helper.hpp>
#include <util/buffer.hpp>

class Parser {

	private: 

		std::ifstream file;

	public:

		Parser(const std::string& path) : file(path) {
			if( !this->file.is_open() ) {
				logger::warn("Failed to open file: '", path, "'!");
			}
		}

		// required so that i can put those in std::map
		Parser() {
			throw std::bad_function_call();
		}

		void load() {
			std::vector<std::string> vec;
			std::string line;

			int counter = 1;

			while( getline(this->file, line) ) {
				split(vec, line, ' ');

				if(vec[0] != "#" && vec.size() > 0) {
					this->handle(vec, counter);
				}

				counter ++;
				vec.clear();
			}
		}

	protected:

		virtual void handle( std::vector<std::string>& parts, int line ) {
			// do nothing by default
		}

};

