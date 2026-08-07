#include "P2Clock.hpp"
#include "P2Time.hpp"

namespace P2
{
	void Clock::start() noexcept
	{
		savedTimePoint = UnderlyingClock::now();
	}

	Time Clock::restart() noexcept
	{
		Time elapsedTime = getElapsedTime();
		savedTimePoint = UnderlyingClock::now();
		return elapsedTime;
	}

	Time Clock::getElapsedTime() const noexcept
	{
		using Duration = std::chrono::duration<UnderlyingClock::rep, UnderlyingClock::period>;

		const auto difference = UnderlyingClock::now() - savedTimePoint;
		const auto asNanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(difference);
		Time time{ asNanoseconds };
		return time;
	}
}