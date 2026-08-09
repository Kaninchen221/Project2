
#include <gtest/gtest.h>

#include "P2GTestFailureSink.hpp"

int main(int argc, char** argv) 
{
    auto gtestFailureSink = std::make_shared<P2::tests::GTestFailureSink<>>();
    
    spdlog::apply_all(
        [&sink = gtestFailureSink]
        (const auto& logger) 
        {
            logger->sinks().push_back(sink);
        }
    );

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}