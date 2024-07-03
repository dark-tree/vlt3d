#pragma once

#if defined(__GNUC__) || defined(__GNUG__)
	// GCC requires C's 'inline' after GNU's 'always_inline'
	#define FORCE_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
	// https://learn.microsoft.com/en-us/cpp/cpp/inline-functions-cpp
	#define FORCE_INLINE __forceinline
#else
	// fallback to C inline
	#define FORCE_INLINE inline
#endif
