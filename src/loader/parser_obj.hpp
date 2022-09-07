#pragma once

#include "parser.hpp"
#include "library.hpp"
#include <renderer/material.hpp>

class MtlParser;

struct Mesh {
	FastBuffer<float> buffer;
	Material material;
};

using MeshLibrary = ObjectLibrary<Mesh>;

class ObjParser : public Parser {

	private:

		FastBuffer<glm::vec3> v;
		FastBuffer<glm::vec3> n;
		FastBuffer<glm::vec2> t;
		std::string name = "default";

		MeshLibrary library;
		Mesh* current = nullptr;

		MtlParser* lib = nullptr;

		float vertex[8];
		int offset[3];
		int vsize = 8;

		static glm::vec3 cast3f( std::vector<std::string>& parts, const int offset ) {
			return glm::vec3( std::stof(parts[offset + 0]), std::stof(parts[offset + 1]), std::stof(parts[offset + 2]) );
		}

		static glm::vec2 cast2f( std::vector<std::string>& parts, const int offset ) {
			return glm::vec2( std::stof(parts[offset + 0]), std::stof(parts[offset + 1]) );
		}

		void face( int type, int index ) {
			switch(type) {
				case 0: memcpy( this->vertex + offset[type], v.data(index - 1), 3 * sizeof(float) ); break; // v
				case 1: memcpy( this->vertex + offset[type], t.data(index - 1), 2 * sizeof(float) ); break; // t
				case 2: memcpy( this->vertex + offset[type], n.data(index - 1), 3 * sizeof(float) ); break; // n
			}
		}

	public:

		ObjParser(const std::string& path) : Parser(path) {
			this->current = library.get(name);
			this->format(0, 3, 5);
		}

		// required so that i can put those in std::map
		ObjParser() {
			throw std::bad_function_call();
		}

		Mesh& get(const std::string& name) {
			return *library.get(name);
		}

		Mesh& get() {
			return *current;
		}

		void format(int v, int t, int n, int size = 8) {
			offset[0] = v;
			offset[1] = t;
			offset[2] = n;
			vsize = size;
		}

	protected:

		void handle( std::vector<std::string>& parts, int line );

};

