
#include "parser_obj.hpp"
#include "resource.hpp"

void ObjParser::handle( std::vector<std::string>& parts, int line ) {
	const std::string& c = parts[0];
	const int args = parts.size() - 1;

	if( c == "o" && args == 1 ) {
		name = parts[1];
		this->current = library.get(name);
	}else

	if( c == "v" && args == 3 ) {
		v.insert( cast3f(parts, 1) );
	}else

	if( c == "vn" && args == 3 ) {
		n.insert( cast3f(parts, 1) );
	}else

	if( c == "vt" && args == 2 ) {
		t.insert( cast2f(parts, 1) );
	}else

	if( c == "f" && args == 3 ) {
		std::vector<std::string> vec;

		for( int i = 1; i <= 3; i ++ ) {
			split(vec, parts[i], '/');

			for( int j = 0; j < 3 && j < vec.size(); j ++ ) {
				face(j, std::stoi(vec[j]));
			}

			current->buffer.insert(this->vertex, vsize);
			vec.clear();
		}
	}else

	if( c == "mtllib" && args == 1 ) {
		this->lib = &Resource::material(parts[1]);
	}else

	if( c == "usemtl" && args == 1 && lib != nullptr ) {
		current->material = lib->get(parts[1]);
	}else

	logger::warn("Invalid or unsuported OBJ command at line: ", line);
}
