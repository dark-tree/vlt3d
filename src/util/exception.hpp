#pragma once

#include "external.hpp"

#if __has_include(<source_location>)
#   include <source_location>
#endif

#if __has_include(<stacktrace>)
#   include <stacktrace>
#endif

class StackCollector {
#ifdef __cpp_lib_stacktrace

	private:

        std::set<std::stacktrace_entry> entries;

        std::string filterPath(std::string path);
        std::string filterName(std::string name);

    public:

        void print(const std::stacktrace& stack, bool cull);

#endif
};

class TracePoint {

	private:

#ifdef __cpp_lib_source_location
		std::source_location source;
#endif

#ifdef __cpp_lib_stacktrace
		std::stacktrace trace;
#endif

	public:

		TracePoint(const TracePoint&) = default;
		TracePoint(TracePoint&&) = default;

#if defined(__cpp_lib_stacktrace)
		TracePoint(const std::source_location source = std::source_location::current(), const std::stacktrace trace = std::stacktrace::current());
		static TracePoint current(const std::source_location source = std::source_location::current(), const std::stacktrace trace = std::stacktrace::current());
#elif defined(__cpp_lib_source_location)
		TracePoint(const std::source_location source = std::source_location::current());
		static TracePoint current(const std::source_location source = std::source_location::current());
#else
		TracePoint() = default;
		static TracePoint current();
#endif

	public:

		size_t getLine() const;
		size_t getColumn() const;
		std::string getPath() const;
		std::string getFunction() const;
		std::string getLocation() const;

#ifdef __cpp_lib_stacktrace
		const std::stacktrace& getStacktrace() const;
#endif

	public:

		void print(StackCollector& collector) const;

};

class Exception : public std::exception {

	private:

		static constexpr const char* chain_str = "Exception chain caused by: ";
		static constexpr const char* cause_str = "Caused by";
		static constexpr const char* throw_str = "Exception";

		TracePoint trace;
		std::string message, reason;
		std::shared_ptr<Exception> cause;

		void print(StackCollector& collector, bool top) const;

	public:

		Exception() = default;
		Exception(const Exception&) = default;
		Exception(Exception&&) = default;

		Exception(const std::string message, Exception& other, const TracePoint trace = TracePoint::current());
		Exception(const std::string message, const TracePoint trace = TracePoint::current());

		const char* what() const noexcept override;
		void print() const;

		bool isPrimary() const;
		const TracePoint& getTrace() const;
		const std::string& getMessage() const;
		std::string getFunction() const;
		std::string getLocation() const;

};