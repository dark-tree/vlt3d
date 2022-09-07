
#include "parser_mtl.hpp"
#include "resource.hpp"

void MtlParser::handle( std::vector<std::string>& parts, int line ) {
	const std::string& c = parts[0];
	const int args = parts.size() - 1;

	if( c == "newmtl" && args == 1 ) {
		name = parts[1];
		this->current = library.get(name);
	}else

	if( c == "Ns" && args == 1 ) {
		this->current->shininess = std::stof(parts[1]);
	}else

	if( c == "map_Ks" && args == 1 ) {
		this->current->specular_map = Resource::texture(parts[1]);
	}else

	if( c == "map_Ke" && args == 1 ) {
		this->current->emissive_map = Resource::texture(parts[1]);
	}else

	if( c == "map_Kd" && args == 1 ) {
		this->current->diffuse_map = Resource::texture(parts[1]);
	}else

	logger::warn("Invalid or unsuported MTL command at line: ", line);
}
