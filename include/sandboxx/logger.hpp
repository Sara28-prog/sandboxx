#ifndef SANDBOXX_LOGGER_HPP
#define SANDBOXX_LOGGER_HPP

#include <string>
#include <mutex>
#include <fstream>
#include <iostream>

namespace sandboxx {

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    FATAL = 4
};

class Logger {
public:
    static Logger& instance();

    void init(const std::string& log_file = "", LogLevel min_level = LogLevel::INFO);
    void log(LogLevel level, const std::string& file, int line, const std::string& message);
    void set_min_level(LogLevel level);

private:
    Logger() = default;
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::mutex mutex_;
    LogLevel min_level_{LogLevel::INFO};
    std::ofstream file_stream_;
    bool has_file_{false};

    std::string level_to_string(LogLevel level) const;
    std::string get_current_timestamp() const;
};

#define LOG_DEBUG(msg) ::sandboxx::Logger::instance().log(::sandboxx::LogLevel::DEBUG, __FILE__, __LINE__, msg)
#define LOG_INFO(msg)  ::sandboxx::Logger::instance().log(::sandboxx::LogLevel::INFO,  __FILE__, __LINE__, msg)
#define LOG_WARN(msg)  ::sandboxx::Logger::instance().log(::sandboxx::LogLevel::WARN,  __FILE__, __LINE__, msg)
#define LOG_ERROR(msg) ::sandboxx::Logger::instance().log(::sandboxx::LogLevel::ERROR, __FILE__, __LINE__, msg)
#define LOG_FATAL(msg) ::sandboxx::Logger::instance().log(::sandboxx::LogLevel::FATAL, __FILE__, __LINE__, msg)

} // namespace sandboxx

#endif // SANDBOXX_LOGGER_HPP
