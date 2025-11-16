#pragma once

#include "Logger.hpp"

#define LOG_TRACE logger::Logger::instance().trace() << "[" << __FILE__ << ":" << __LINE__ << " " << __func__ << "] "
#define LOG_DEBUG logger::Logger::instance().debug() << "[" << __FILE__ << ":" << __LINE__ << " " << __func__ << "] "
#define LOG_INFO logger::Logger::instance().info()   << "[" << __FILE__ << ":" << __LINE__ << " " << __func__ << "] "
#define LOG_WARN logger::Logger::instance().warn()   << "[" << __FILE__ << ":" << __LINE__ << " " << __func__ << "] "
#define LOG_ERROR logger::Logger::instance().error() << "[" << __FILE__ << ":" << __LINE__ << " " << __func__ << "] "
#define LOG_FATAL logger::Logger::instance().fatal() << "[" << __FILE__ << ":" << __LINE__ << " " << __func__ << "] "