#pragma once

/// Define platform and compiler specific macros
#ifdef _MSVC_LANG

#	define P2_WINDOWS 1
#	define P2_MSVC 1

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

#	define P2_TIME_TRACE P2_DEBUG

#endif

/// It's used in 99% headers
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

/// Types used across the entire lib
namespace P2
{
	constexpr inline static size_t InvalidIndex = std::numeric_limits<size_t>::max();

	using ID = size_t;
	using TypeID = size_t;
	constexpr inline static ID InvalidID = InvalidIndex;

	using ThreadID = uint8_t;
}