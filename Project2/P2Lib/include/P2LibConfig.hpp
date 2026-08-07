
/// Define Platform
#ifdef _MSVC_LANG

#	define P2_WINDOWS 1

#else

#	error "Unsupported platform"

#endif /// _MSVC_LANG

/// Config Lib for windows platform
#ifdef P2_WINDOWS

#	if _DEBUG
#		define P2_DEBUG 1
#		define P2_RELEASE 0
#	else
#		define P2_DEBUG 0
#		define P2_RELEASE 1
#	endif /// _DEBUG

/// Empty because we suppport only "Static" lib
#	define P2_API

#endif