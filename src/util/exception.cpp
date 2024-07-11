
#include "exception.hpp"

#ifdef __cpp_lib_stacktrace
std::string StackCollector::filterPath(std::string path) {
	//TODO
	//#ifdef _MSC_VER
	//return std::filesystem::path(path).lexically_relative("").generic_string();
	//#endif

	return path;
}

std::string StackCollector::filterName(std::string name) {
	#ifdef _MSC_VER
	static std::regex msvc_name ("^(.+?!)?(.+?)(\\+.+)?$");
	std::smatch matches;

	if(std::regex_search(name, matches, msvc_name)) {
		return matches.size() >= 3 ? matches[2].str() : name;
	}
	#endif

	return name;
}

void StackCollector::print(const std::stacktrace& stack, bool cull) {
	int limit = -1;
	int skipped = 0;
	bool search = true;

	if (stack.size() == 0) {
		return;
	}

	if (cull) {
		for (int i = stack.size() - 1; i >= 0; i --) {
			if (search) {
				if (!entries.contains(stack[i])) {
					search = false;
					limit = i;
				} else skipped ++;
			}

			entries.insert(stack[i]);
		}
	} else {
		limit = stack.size() - 1;
	}

	for (int i = 0; i <= limit; i ++) {
		const std::stacktrace_entry& entry = stack[i];
		const std::string path = filterPath(entry.source_file());
		const std::string name = filterName(entry.description());
		int line = entry.source_line();

		if (!entry || path.empty()) {
			printf("       at: ??? %s\n", name.c_str());
			continue;
		}

		printf("       at: %s:%d %s\n", path.c_str(), line, name.c_str());
	}

	if (skipped) {
		printf("       ... %d more\n", skipped);
	}
}
#endif

////////////////////////
/// TracePoint class ///
////////////////////////

#if defined(__cpp_lib_stacktrace)
TracePoint::TracePoint(const std::source_location source, const std::stacktrace trace)
: source(source), trace(trace) {}

TracePoint TracePoint::current(const std::source_location source, const std::stacktrace trace) {
	return {source, trace};
}
#elif defined(__cpp_lib_source_location)
TracePoint::TracePoint(const std::source_location source)
: source(source) {}

TracePoint TracePoint::current(const std::source_location source) {
	return {source};
}
#else
TracePoint TracePoint::current() {
	return {};
}
#endif

size_t TracePoint::getLine() const {
	#ifdef __cpp_lib_source_location
	return source.line();
	#else
	return 0;
	#endif
}

size_t TracePoint::getColumn() const {
	#ifdef __cpp_lib_source_location
	return source.column();
	#else
	return 0;
	#endif
}

std::string TracePoint::getPath() const {
	#ifdef __cpp_lib_source_location
	return source.file_name();
	#else
	return "";
	#endif
}

std::string TracePoint::getFunction() const {
	#ifdef __cpp_lib_source_location
	return source.function_name();
	#else
	return "";
	#endif
}

std::string TracePoint::getLocation() const {
	#ifdef __cpp_lib_source_location
	return getPath() + ":" + std::to_string(getLine()) + ":" + std::to_string(getColumn());
	#else
	return "???";
	#endif
}

#ifdef __cpp_lib_stacktrace
const std::stacktrace& TracePoint::getStacktrace() const {
	return trace;
}
#endif

void TracePoint::print(StackCollector& collector) const {
	printf("     from: %s\n", getLocation().c_str());

	#ifdef __cpp_lib_stacktrace
	collector.print(trace, true);
	#endif
}

///////////////////////
/// Exception class ///
///////////////////////

void Exception::print(StackCollector& collector, bool top) const {
	printf("%s: %s\n", top ? throw_str : cause_str, message.c_str());
	trace.print(collector);

	if (cause) {
		cause->print(collector, false);
	}
}

Exception::Exception(const std::string message, Exception& other, const TracePoint trace)
: trace(trace), message(message), reason(other.isPrimary() ? chain_str + other.reason : other.reason), cause(std::make_shared<Exception>(other)) {}

Exception::Exception(const std::string message, const TracePoint trace)
: trace(trace), message(message), reason(getLocation() + ": " + message), cause(nullptr) {}

const char* Exception::what() const noexcept {
	return reason.c_str();
}

void Exception::print() const {
	StackCollector collector;
	print(collector, true);
}

bool Exception::isPrimary() const {
	return !cause;
}

const TracePoint& Exception::getTrace() const {
	return trace;
}

const std::string& Exception::getMessage() const {
	return message;
}

std::string Exception::getFunction() const {
	return trace.getFunction();
}

std::string Exception::getLocation() const {
	return trace.getLocation();
}