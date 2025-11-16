#include "Logger.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <iterator>
#include <mutex>
#include <optional>
#include <string_view>
#include <cstdlib>

namespace {
std::optional<logger::LogLevel> upper(std::string_view level){
        std::string upper_str;
        upper_str.reserve(level.length());
        std::transform(level.begin(), level.end(), std::back_inserter(upper_str),
            [](unsigned char c) {return std::toupper(c);});
        if (upper_str == "TRACE") {
            return logger::LogLevel::Trace;
        } else if (upper_str == "DEBUG") {
            return logger::LogLevel::Debug;
        } else if (upper_str == "INFO") {
            return logger::LogLevel::Info;
        } else if (upper_str == "WARN") {
            return logger::LogLevel::Warn;
        } else if (upper_str == "ERROR") {
            return logger::LogLevel::Error;
        } else if (upper_str == "FATAL") {
            return logger::LogLevel::Fatal;
        } else {
            return std::nullopt;
        }
    }
}

namespace  logger {

    Logger& Logger::instance() noexcept {
        static Logger log;
        return log;
    }

    void Logger::set_level(LogLevel level) noexcept{
        level_.store(level);
    }

    LogLevel Logger::level() const noexcept {
        return level_.load();
    }

    LogStream Logger::trace() noexcept{
        return LogStream(*this, LogLevel::Trace);
    }
    LogStream Logger::debug() noexcept{
        return LogStream(*this, LogLevel::Debug);
    }
    LogStream Logger::info() noexcept{
        return LogStream(*this, LogLevel::Info);
    }
    LogStream Logger::warn() noexcept{
        return LogStream(*this, LogLevel::Warn);
    }
    LogStream Logger::error() noexcept{
        return LogStream(*this, LogLevel::Error);
    }
    LogStream Logger::fatal() noexcept{
        return LogStream(*this, LogLevel::Fatal);
    }


    void Logger::write(LogLevel level, const std::string& message){
        LogLevel current_level = level_.load();
        
        if (static_cast<int>(level) < static_cast<int>(current_level))
            return;
        std::string level_string;
        std::lock_guard<std::mutex> guard(mutex_);
        switch (level) {
        case LogLevel::Trace:
            level_string = "[Trace] ";
            break;
        case LogLevel::Debug:
            level_string = "[Debug] ";
            break;
        case LogLevel::Info:
            level_string = "[Info] ";
            break;
        case LogLevel::Warn:
            level_string = "[Warn] ";
            break;
        case LogLevel::Error:
            level_string = "[Error] ";
            break;
        case LogLevel::Fatal:
            level_string = "[Fatal] ";
            break;
        }
        std::clog<< level_string << message <<"\n";
    }

    bool Logger::load_env_level() noexcept{
        char* env_level = std::getenv("LOG_LEVEL");
        if(env_level != nullptr){
            std::string_view str_level(env_level);
            auto new_log_level = upper(str_level);
            set_level(new_log_level.value());
            return true;
        }
        return false;
    }

    LogStream::LogStream(Logger& logger, LogLevel level) : 
        logger_(logger), level_(level){}

    LogStream::~LogStream() noexcept{
        logger_.write(level_, buffer_.str());
    }

}