#pragma once

namespace trait {

	// https://stackoverflow.com/a/53967057
	template <typename T, typename = void>
	struct is_iterable : std::false_type {};

	template <typename T>
	struct is_iterable<T, std::void_t<decltype(std::begin(std::declval<T&>())), decltype(std::end(std::declval<T&>()))>> : std::true_type {};

	template<typename T>
	concept IterableContainer = is_iterable<T>::value;

	template <typename T>
	concept ConvertibleToStdString = requires(T a){ std::to_string(a); };

	template <typename T>
	concept CastableToStdString = std::convertible_to<T, std::string>;

}
