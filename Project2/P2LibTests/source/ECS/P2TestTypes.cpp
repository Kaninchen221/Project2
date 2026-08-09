#include "P2TestTypes.hpp"

#include "ECS/P2Query.hpp"
#include "ECS/P2World.hpp"

namespace P2::tests
{
	namespace TestSystemIncrementer
	{
		void entryPoint(ecs::Query<Counter> counters)
		{
			for (auto [counter] : counters)
			{
				counter->value++;
			}
		}
	}
}