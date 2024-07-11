#pragma once

#include "trait.hpp"

namespace util {

	template<typename A, typename B>
	using Comparator = bool (*) (const A&, const B&);

	// get element from map, ar return the default value
	template<template<class, class, class...> class C, typename K, typename V, typename... Args>
	const V& fallback_get(const C<K, V, Args...>& m, K const& key, const V& fallback) {
		typename C<K, V, Args...>::const_iterator it = m.find(key);
		return (it == m.end()) ? fallback : it->second;
	}

	// check if collection contains the element
	template<trait::IterableContainer T, typename V>
	bool contains(const T& collection, const V& needle, Comparator<V, V> comparator = +[] (const V& a, const V& b) -> bool { return a == b; }) {
		for (auto& value : collection) {
			if (comparator(value, needle)) return true;
		}

		return false;
	}

	// convert any value (including std::string and const char*) to std::string
	template <typename T>
	inline std::string any_to_string(const T& value) {
		return "<non-printable value>";
	}

	template <trait::ConvertibleToStdString T>
	inline std::string any_to_string(const T& value) {
		return std::to_string(value);
	}

	template <trait::CastableToStdString T>
	inline std::string any_to_string(const T& value) {
		return value;
	}

}
