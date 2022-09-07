
template< typename T >
struct DefaultAllocator {

	static T* realloc(T* buffer, int length) {
		return (T*) ::realloc(buffer, length * sizeof(T));
	}
	
	static T* malloc(int length) {
		return (T*) ::malloc(length * sizeof(T));
	}

};

