
#constant USE_FALLBACK_STORAGE
#constant MAX_BUFFER_LENGTH

#ifdef USE_FALLBACK_STORAGE
#	define BUFFER_BLOCK(index) layout (binding = index) uniform
#	define UNLIMITED_ARRAY(name) name[MAX_BUFFER_LENGTH]
#else
#	define BUFFER_BLOCK(index) layout (std430, binding = index) buffer
#	define UNLIMITED_ARRAY(name) name[]
#endif

