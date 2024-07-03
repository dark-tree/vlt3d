#pragma once

#include "external.hpp"

/**
 * Start a timer on class construction
 * each call to the milliseconds() or nanoseconds()
 * will return the current elapsed time from that initial point
 *
 * Example usage:
 *
 * if (Timer timer; timer) {
 *     // some operation
 *     logger::info("Operation took: ", timer.milliseconds(), "ms");
 * }
 */
class Timer {

	private:

		typedef std::chrono::high_resolution_clock Clock;
		READONLY Clock::time_point start;

		template <typename T>
		std::chrono::duration<double, T> elapsed() const {
			return std::chrono::duration<double, T> {Clock::now() - start};
		}

	public:

		Timer()
		: start(Clock::now()) {}

		double milliseconds() const {
			return elapsed<std::milli>().count();
		}

		double nanoseconds() const {
			return elapsed<std::nano>().count();
		}

		operator bool() const {
			return true;
		}

		// experimental helper
		static inline Timer of(const std::function<void()>& thing) {
			Timer timer;
			thing();
			return timer;
		}

};
