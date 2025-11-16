#pragma once

#include <atomic>
#include <mutex>
#include <sstream>
#include <string>

namespace logger {

    enum class LogLevel{
        Trace,
        Debug,
        Info,
        Warn,
        Error,
        Fatal
    };

    class LogStream;

    class Logger {

    private:
        std::mutex mutex_;

        friend class LogStream;

        std::atomic<LogLevel> level_ = LogLevel::Info;

        Logger () noexcept = default;

        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
        Logger(Logger&&) = delete;
        Logger& operator=(Logger&&) = delete;

        void write(LogLevel level, const std::string& message);

    public: 

        static Logger& instance() noexcept;

        void set_level(LogLevel level) noexcept;

        LogLevel level() const noexcept;


        LogStream trace() noexcept;
        LogStream debug() noexcept;
        LogStream info() noexcept;
        LogStream warn() noexcept;
        LogStream error() noexcept;
        LogStream fatal() noexcept;   
        
        bool load_env_level() noexcept;

    };

    class LogStream{
    private: 
        Logger& logger_;
        LogLevel level_;

        std::ostringstream buffer_;

    public:
        LogStream(Logger& logger, LogLevel level);
        ~LogStream() noexcept;

        template<typename T>
        LogStream& operator<<(const T& value) {
            buffer_ << value;
            return *this;
        };
    };
}