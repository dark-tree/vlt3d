#pragma once

template< typename T >
class ObjectLibrary {

	private:

		std::unordered_map<std::string, T> objects;

	public:

		T* get(const std::string& key) {
			return &(objects[key]);
		}

};


