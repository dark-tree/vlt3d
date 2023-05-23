#pragma once

#include "util/util.hpp"
#include "util/logger.hpp"

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

		void orFail() {
			if (!successful) throw std::runtime_error(what());
		}

		void orWarn() {
			if (!successful) logger::warn(what());
		}

};
