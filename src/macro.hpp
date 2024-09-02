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

#ifdef __GNUC__ // GCC 4.8+, Clang, Intel and other compilers compatible with GCC (-std=c++0x or above)
#	define UNREACHABLE __builtin_unreachable()
#elif defined(_MSC_VER) // MSVC
#	define UNREACHABLE __assume(false)
#else
#	define UNREACHABLE std::terminate()
#endif
