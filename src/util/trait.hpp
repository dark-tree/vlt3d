#pragma once

namespace trait {
	
	/// internal utility, check if the given types are equal
	template<class Type, class... Args>
	using are_of_type = std::conjunction<std::is_same<Type, Args>...>;

	/// internal utility, asserts that given types are equal
	template<class Type, class... Args>
	using are_types_equal = std::enable_if_t<are_of_type<Type, Args...>::value, void>;

}
