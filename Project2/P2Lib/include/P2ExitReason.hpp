#pragma once

#include "P2LibConfig.hpp"

#include <string>

namespace P2
{
	struct P2_API ExitReason
	{
		enum Level
		{
			Info,
			Error,
			Critical
		};

		std::string reason;
		Level level = Error;
	};
}