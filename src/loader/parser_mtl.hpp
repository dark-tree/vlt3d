#pragma once

#include "parser.hpp"
#include "library.hpp"
#include <renderer/material.hpp>

using MaterialLibrary = ObjectLibrary<Material>;

class MtlParser : public Parser {

	private:

		std::string name = "default";

		MaterialLibrary library;
		Material* current = nullptr;

	public:

		MtlParser(const std::string& path) : Parser(path) {
			this->current = library.get(path);
		}

		// required so that i can put those in std::map
		MtlParser() {
			throw std::bad_function_call();
		}

		Material& get(const std::string& name) {
			return *library.get(name);
		}

		Material& get() {
			return *current;
		}

	protected:

		void handle( std::vector<std::string>& parts, int line );

};

