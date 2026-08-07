#pragma once

#include "P2LibConfig.hpp"

#include <chrono>


namespace P2
{	
	class P2_API Time 
	{

	public:
		using Nanoseconds = std::chrono::nanoseconds;
		using Microseconds = std::chrono::microseconds;
		using Milliseconds = std::chrono::milliseconds;
		using Seconds = std::chrono::seconds;
		using Minutes = std::chrono::minutes;
		using Hours = std::chrono::hours;

		Time() noexcept;
		Time(const Time& other) noexcept = default;
		Time(Time&& other) noexcept = default;
		Time(Nanoseconds nanoseconds) noexcept;

		Time& operator = (const Time& other) noexcept = default;
		Time& operator = (Time&& other) noexcept = default;
		Time& operator = (Nanoseconds nanoseconds) noexcept;

		~Time() noexcept = default;

		Nanoseconds getAsNanoseconds() const noexcept;
		Microseconds getAsMicroseconds() const noexcept;
		Milliseconds getAsMilliseconds() const noexcept;
		Seconds getAsSeconds() const noexcept;
		Minutes getAsMinutes() const noexcept;
		Hours getAsHours() const noexcept;

		static Time FromNanoseconds(Nanoseconds nanoseconds) noexcept;
		static Time FromMicroseconds(Microseconds microseconds) noexcept;
		static Time FromMilliseconds(Milliseconds milliseconds) noexcept;
		static Time FromSeconds(Seconds seconds) noexcept;
		static Time FromMinutes(Minutes minutes) noexcept;
		static Time FromHours(Hours hours) noexcept;

		template<class SourceT>
		static Time From(SourceT&& source);

		auto operator <=> (const Time& other) const noexcept = default;

		friend Time operator - (const Time& first, const Time& second) noexcept;
		Time& operator -= (const Time& other) noexcept;

		friend Time operator + (const Time& first, const Time& second) noexcept;
		Time& operator += (const Time& other) noexcept;

	private:

		Nanoseconds timeAsNanoseconds;
	};

	template<class SourceT>
	Time Time::From(SourceT&& source)
	{
		return std::chrono::duration_cast<SourceT>(source);
	}

	inline Time operator - (const Time& first, const Time& second) noexcept
	{
		return Time{ first.timeAsNanoseconds - second.timeAsNanoseconds };
	}

	inline Time& Time::operator-=(const Time& other) noexcept
	{
		timeAsNanoseconds -= other.timeAsNanoseconds;
		return *this;
	}

	inline Time operator + (const Time& first, const Time& second) noexcept
	{
		return Time{ first.timeAsNanoseconds + second.timeAsNanoseconds };
	}

	inline Time& Time::operator+=(const Time& other) noexcept
	{
		timeAsNanoseconds += other.timeAsNanoseconds;
		return *this;
	}
}