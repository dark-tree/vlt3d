#pragma once

#include <include/external.hpp>
#include <renderer/texture.hpp>

#include "library.hpp"
#include "parser_mtl.hpp"
#include "parser_obj.hpp"

#define LD3D_OBJ_ENFORMAT(v, t, n, s) (v<<0 | t<<8 | n<<16 | s<<24)
#define LD3D_OBJ_DEFORMAT_V(x) (x>>0  & 0xFF)
#define LD3D_OBJ_DEFORMAT_T(x) (x>>8  & 0xFF)
#define LD3D_OBJ_DEFORMAT_N(x) (x>>16 & 0xFF)
#define LD3D_OBJ_DEFORMAT_S(x) (x>>24 & 0xFF)

#define LD3D_OBJ_VTN LD3D_OBJ_ENFORMAT(0, 3, 5, 8)
#define LD3D_OBJ_VNT LD3D_OBJ_ENFORMAT(0, 6, 3, 8)
#define LD3D_OBJ_VT LD3D_OBJ_ENFORMAT(0, 3, 5, 5)

class Resource {

	private:

		static std::unordered_map<std::string, Texture*> textures;
		static std::unordered_map<std::string, MtlParser> mtls;
		static std::unordered_map<std::string, ObjParser> objs;
	
	public:

		static Texture* texture(const std::string& path);
		static MtlParser& material(const std::string& path);
		static ObjParser& object(const std::string& path, long format = LD3D_OBJ_VNT);

		static void clean();

};

