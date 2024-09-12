#pragma once

#include "external.hpp"

/**
 * Starts a timer on class construction
 * each call to the milliseconds() or nanoseconds()
 * will return the current elapsed time from that initial point
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

		Timer();

		/**
		 * Returns the elapsed time in milliseconds
		 */
		double milliseconds() const;

		/**
		 * Returns the elapsed time in nanoseconds
		 */
		double nanoseconds() const;

		/**
		 * Always covert to true, needed to facilitate usage in ifs,
		 * use like this:
		 * @code
		 *
		 * if (Timer timer; timer) {
		 *     // some operation
		 *     logger::info("Operation took: ", timer.milliseconds(), "ms");
		 * }
		 */
		operator bool() const;

		/**
		 * Time the given lambda, returns a still running timer,
		 * use like this:
		 * @code
		 *
		 * logger::info("Operation took: ", Timer::of([&] () {
		 *     // some operation
		 * }).milliseconds(), "ms");
		 */
		static Timer of(const std::function<void()>& thing);

};
