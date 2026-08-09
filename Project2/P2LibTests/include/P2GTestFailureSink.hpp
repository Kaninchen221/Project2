#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>

#include <memory>

namespace P2::tests
{
    template<typename Mutex = std::mutex>
    class GTestFailureSink : public spdlog::sinks::base_sink<Mutex>
    {
        inline static bool IgnoreLog = false;

    public:

        /// Use to not fail tests on log messages. Useful for testing logging itself and when you expect warnings or errors to be logged.
        static void SetIgnoreLog(bool value) noexcept
        {
            IgnoreLog = value;
        }

        static bool GetIgnoreLog() noexcept
        {
            return IgnoreLog;
        }

    protected:

        void sink_it_(const spdlog::details::log_msg& msg) override
        {
            if (GetIgnoreLog())
                return;

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

}