
#include <gtest/gtest.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>

#include <memory>

template<typename Mutex = std::mutex>
class GTestFailureSink : public spdlog::sinks::base_sink<Mutex> 
{
protected:

    void sink_it_(const spdlog::details::log_msg& msg) override 
    {
        switch (msg.level)
        {
        case spdlog::level::warn:
            ADD_FAILURE() << "Logger logged warning";
            return;

        case spdlog::level::err:
            ADD_FAILURE() << "Logger logged error";
            return;

        case spdlog::level::critical:
            ADD_FAILURE() << "Logger logged critical";
            return;
        }
    }

    void flush_() override {}
};

int main(int argc, char** argv) 
{
    auto gtestFailureSink = std::make_shared<GTestFailureSink<>>();
    
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