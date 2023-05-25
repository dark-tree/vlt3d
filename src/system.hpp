#pragma once

#if defined(_WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#	define API_WIN32
#	define GLPH_WINDOWS
#endif

#if defined(__linux__)
#	define API_XLIB
//#	define API_WAYLAND this is not really working so well...
#	define GLPH_LINUX
#endif

#if defined(__APPLE__)
#	warning "The APPLE platform is not supported. Cocoa API is not supported, attempting to build with X11..."
#	define API_XLIB
#	define GLPH_APPLE
#endif
