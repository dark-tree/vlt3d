#pragma once

namespace trait {

	namespace impl {

		// https://stackoverflow.com/a/53967057
		template <typename T, typename = void>
		struct is_iterable : std::false_type {};

		template <typename T>
		struct is_iterable<T, std::void_t<decltype(std::begin(std::declval<T&>())), decltype(std::end(std::declval<T&>()))>> : std::true_type {};

	}

	/// Concept, check if all types in parameter pack are convertible to the common type T
	template<typename T, typename... Ts>
	concept All = (std::convertible_to<Ts, T> && ...);

	/// Concept, check if the given type T can be iterated
	template<typename T>
	concept IterableContainer = impl::is_iterable<T>::value;

	/// Concept, check if the given type T can be converted to std::String with std::to_string
	template <typename T>
	concept ConvertibleToStdString = requires(T a) { std::to_string(a); };

	/// Concept, check if the given type T can be static casted to std::string
	template <typename T>
	concept CastableToStdString = std::convertible_to<T, std::string>;

}
