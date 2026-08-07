#include "P2Time.hpp"

namespace P2
{
	Time::Time() noexcept
		: timeAsNanoseconds(Nanoseconds::zero())
	{

	}

	Time::Time(Nanoseconds nanoseconds) noexcept
		: timeAsNanoseconds(nanoseconds)
	{

	}

	Time& Time::operator=(Nanoseconds nanoseconds) noexcept
	{
		timeAsNanoseconds = nanoseconds;
		return *this;
	}

	Time::Nanoseconds Time::getAsNanoseconds() const noexcept
	{
		return timeAsNanoseconds;
	}

	Time::Microseconds Time::getAsMicroseconds() const noexcept
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(timeAsNanoseconds);
	}

	Time::Milliseconds Time::getAsMilliseconds() const noexcept
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(timeAsNanoseconds);
	}

	Time::Seconds Time::getAsSeconds() const noexcept
	{
		return std::chrono::duration_cast<std::chrono::seconds>(timeAsNanoseconds);
	}

	Time::Minutes Time::getAsMinutes() const noexcept
	{
		return std::chrono::duration_cast<std::chrono::minutes>(timeAsNanoseconds);
	}

	Time::Hours Time::getAsHours() const noexcept
	{
		return std::chrono::duration_cast<std::chrono::hours>(timeAsNanoseconds);
	}

	Time Time::FromNanoseconds(Nanoseconds nanoseconds) noexcept
	{
		return Time(nanoseconds);
	}

	Time Time::FromMicroseconds(Microseconds microseconds) noexcept
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(microseconds);
	}

	Time Time::FromMilliseconds(Milliseconds milliseconds) noexcept
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(milliseconds);
	}

	Time Time::FromSeconds(Seconds seconds) noexcept
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(seconds);
	}

	Time Time::FromMinutes(Minutes minutes) noexcept
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(minutes);
	}

	Time Time::FromHours(Hours hours) noexcept
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(hours);
	}

}
