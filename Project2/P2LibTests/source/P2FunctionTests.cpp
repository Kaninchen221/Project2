#pragma once

#include "P2Function.hpp"

#include <gtest/gtest.h>

namespace P2::tests
{
    class FunctionTests : public ::testing::Test
    {
    protected:

        static int MarkAndReturnI(int i) { fooInvoked = true; return i; }

        inline static bool fooInvoked = false;
	};

	namespace FunctionTestsTypes
	{
		void simpleFunc() {}
	}

	TEST_F(FunctionTests, CreateFromEmptyLambdaTest)
	{
		auto lambda = []() {};
		Function<void> function{ lambda };
		function = lambda;
	}

	TEST_F(FunctionTests, AsParamInFunctionTest)
	{
		using namespace FunctionTestsTypes;

		auto lambdaWithFunctionAsParam = []([[maybe_unused]] Function<void> function) {};
		lambdaWithFunctionAsParam(simpleFunc);

		auto simpleLambda = []() {};
		lambdaWithFunctionAsParam(Function<void>(simpleLambda));
		lambdaWithFunctionAsParam(simpleLambda);
	}

	TEST_F(FunctionTests, ComplexTest)
	{
		auto lambda = [](int) -> int { return {}; };
		Function<int, int> function{ lambda };

		ASSERT_EQ(function.getInternalFunction(), lambda);
		ASSERT_TRUE(function.isValid());
		ASSERT_TRUE(function.operator bool());

		function.invalidate();

		ASSERT_FALSE(function.isValid());

		function = MarkAndReturnI;
		ASSERT_TRUE(function.isValid());

		const int expectedValue = 0;

		const auto& constFunction = function;
		const int actualValue = constFunction.invoke(expectedValue);
		ASSERT_EQ(expectedValue, actualValue);
	}
}