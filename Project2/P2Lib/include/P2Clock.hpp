#pragma once

#include "P2LibConfig.hpp"
#include "P2Time.hpp"

#include <chrono>

namespace P2
{
	class P2_API Clock
	{
	public:

		using UnderlyingClock = std::chrono::system_clock;

		Clock() noexcept { start(); }
		Clock(const Clock& other) noexcept = default;
		Clock(Clock&& other) noexcept = default;
		~Clock() noexcept = default;

		Clock& operator = (const Clock& other) noexcept = default;
		Clock& operator = (Clock&& other) noexcept = default;

		void start() noexcept;
		Time restart() noexcept;
		Time getElapsedTime() const noexcept;

	private:

		UnderlyingClock::time_point savedTimePoint;

	};

}