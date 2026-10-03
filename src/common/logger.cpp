#include "sandboxx/logger.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace sandboxx {

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

Logger::~Logger() {
    if (file_stream_.is_open()) {
        file_stream_.close();
    }
}

void Logger::init(const std::string& log_file, LogLevel min_level) {
    std::lock_guard<std::mutex> lock(mutex_);
    min_level_ = min_level;
    if (!log_file.empty()) {
        file_stream_.open(log_file, std::ios::out | std::ios::app);
        has_file_ = file_stream_.is_open();
    }
}

void Logger::set_min_level(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    min_level_ = level;
}

std::string Logger::level_to_string(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

std::string Logger::get_current_timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

void Logger::log(LogLevel level, const std::string& file, int line, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (level < min_level_) return;

    std::string timestamp = get_current_timestamp();
    std::string level_str = level_to_string(level);

    // Extract basename of file
    size_t last_slash = file.find_last_of("/\\");
    std::string short_file = (last_slash != std::string::npos) ? file.substr(last_slash + 1) : file;

    std::stringstream ss;
    ss << "[" << timestamp << "] [" << level_str << "] [" << short_file << ":" << line << "] " << message;

    if (level >= LogLevel::ERROR) {
        std::cerr << ss.str() << std::endl;
    } else {
        std::cout << ss.str() << std::endl;
    }

    if (has_file_) {
        file_stream_ << ss.str() << std::endl;
        file_stream_.flush();
    }
}

} // namespace sandboxx
