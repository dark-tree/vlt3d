
#include "resource.hpp"
#include "parser_mtl.hpp"
#include "parser_obj.hpp"

std::unordered_map<std::string, Texture*> Resource::textures;
std::unordered_map<std::string, MtlParser> Resource::mtls;
std::unordered_map<std::string, ObjParser> Resource::objs;

Texture* Resource::texture(const std::string& path) {

	const std::string& key = std::filesystem::canonical(path).string();
	auto it = textures.find(key);

	if( it != textures.end() ) {
		return it->second;
	}

	int w, h, n;
	byte* data = stbi_load(path.c_str(), &w, &h, &n, 4);

	if( data == nullptr ) {
		throw std::runtime_error("Failed to load image: '" + std::string(path) + "'");
	}

	logger::info("Loading resource: ", key);

	Texture* texture = new Texture(w, h, GL_RGBA, GL_RGBA);
	texture->update(data);
	stbi_image_free(data);
	
	textures[key] = texture;

	return texture;
}

MtlParser& Resource::material(const std::string& path) {
	
	const std::string& key = std::filesystem::canonical(path).string();
	auto it = mtls.find(key);

	if( it != mtls.end() ) {
		return it->second;
	}

	logger::info("Loading resource: ", key);

	mtls.emplace(key, path);
	mtls[key].load();

	return mtls[key];
}

ObjParser& Resource::object(const std::string& path, long format) {
	
	const std::string& key = std::filesystem::canonical(path).string();
	auto it = objs.find(key);

	if( it != objs.end() ) {
		return it->second;
	}

	logger::info("Loading resource: ", key);

	objs.emplace(key, path);
	objs[key].format(
		LD3D_OBJ_DEFORMAT_V(format),
		LD3D_OBJ_DEFORMAT_T(format),
		LD3D_OBJ_DEFORMAT_N(format),
		LD3D_OBJ_DEFORMAT_S(format)
	);
	objs[key].load();

	return objs[key];
}

void Resource::clean() {

	for( const auto& pair : textures ) {
		delete pair.second;
	}

	textures.clear();
	mtls.clear();
	objs.clear();
}

