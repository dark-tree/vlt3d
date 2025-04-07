#pragma once

/**
 * If a class exposes a public variable that cannot or shouldn't be modified
 * by the external code prefix its type with this usage hint, this is useful
 * when avoiding a `const` keyword on fields of assignable classes
 */
#define READONLY

/**
 * By default in VLT3D all raw pointer function arguments are assumed as
 * non-null, if you are writing a function that expects a nullable pointer as
 * argument prefix the argument type with this hint
 */
#define NULLABLE

/**
 * Can by use in function bodies to mark a spot that should normally be unreachable
 * during execution, for example after an enum switch. Sometimes needed when
 * the '-Werror=return-type' flag is used as the compiler is not always smart enough
 */
#ifdef __GNUC__ // GCC 4.8+, Clang, Intel and other compilers compatible with GCC (-std=c++0x or above)
#	define UNREACHABLE __builtin_unreachable()
#elif defined(_MSC_VER) // MSVC
#	define UNREACHABLE __assume(false)
#else
#	define UNREACHABLE std::terminate()
#endif

/**
 * Can be used in place of normal 'inline' to force the compiler's
 * hand a little and actually guarantee that the annotated function
 * will be inlined into the caller.
 */
#ifndef NDEBUG
#	define FORCE_INLINE inline
#else
#	if defined(__GNUC__) || defined(__GNUG__)
//		GCC requires C's 'inline' after GNU's 'always_inline'
#		define FORCE_INLINE __attribute__((always_inline)) inline
#	elif defined(_MSC_VER)
//		https://learn.microsoft.com/en-us/cpp/cpp/inline-functions-cpp
#		define FORCE_INLINE __forceinline
#	else
//		fallback to C inline
#		define FORCE_INLINE inline
#	endif
#endif
