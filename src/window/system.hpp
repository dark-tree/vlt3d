#pragma once

#if defined(_WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#	define API_WIN32
#endif

#if defined(__linux__)
#	define API_X11
#	define API_WAYLAND
#endif

#if defined(__APPLE__)
#	warning "The APPLE platform is not supported. Cocoa API is not supported, attempting to build with X11..."
#	define API_X11
#endif
