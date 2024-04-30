#pragma once

#include "util/util.hpp"
#include "util/logger.hpp"

/**
 * An optional-like class representing the result of certain operations
 */
template<typename T>
class Result {

	private:

		const T value;
		const bool successful;

		std::string what() {
			return util::any_to_string(value);
		}

	public:

		Result(const T& value, bool successful)
		: value(value), successful(successful) {}

		operator bool() const {
			return successful;
		}

		/**
		 * If the result is unsuccessful throw an error
		 */
		void orFail() {
			if (!successful) throw std::runtime_error(what());
		}

		/**
		 * If the result is unsuccessful log a warning
		 */
		void orWarn() {
			if (!successful) logger::warn(what());
		}

		/**
		 * If the result is unsuccessful execute a callback
		 */
		void orElse(const std::function<void(const T&)>& callback) {
			if (!successful) callback(value);
		}

};
