#pragma once

#include "P2LibConfig.hpp"
#include "P2Logger.hpp"

#include <exception>

#if P2_MSVC
	#include <intrin.h>
#endif // ZINET_MSVC

namespace P2
{

#if P2_DEBUG

	static inline bool Ensure(bool value)
	{
		if (!value)
		{
		#if P2_MSVC
			__nop();
			__debugbreak();
		#endif // P2_MSVC
		}

		return value;
	}

	static inline bool Ensure(bool value, const char* message)
	{
		bool shouldEnsure = !value;

		if (shouldEnsure)
		{
			static auto Logger = ConsoleLogger::CreateOrGet("Ensure");
			Logger->error(message);
		}

		return Ensure(value);
	}
#else
	static inline bool Ensure(bool Value) { return Value; }
	static inline bool Ensure(bool Value, const char* /*message*/) { return Value; }
#endif // ZINET_DEBUG

#if ZINET_DEBUG
	[[noreturn]] inline static void TerminateDebug() noexcept
	{
		std::terminate();
	}

	inline static void TerminateDebug([[maybe_unused]] bool ShouldTerminate) noexcept
	{
		if (ShouldTerminate)
			std::terminate();
	}
#else
	inline static void TerminateDebug() noexcept {}
	inline static void TerminateDebug([[maybe_unused]] bool ShouldTerminate) noexcept {}
#endif // ZINET_DEBUG

	[[noreturn]] inline static void Terminate() noexcept { std::terminate(); }
}