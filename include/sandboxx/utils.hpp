#ifndef SANDBOXX_UTILS_HPP
#define SANDBOXX_UTILS_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace sandboxx {
namespace utils {

// Checks whether current process is running with root / CAP_SYS_ADMIN privileges
bool is_root_user();

// Safe string splitting by delimiter
std::vector<std::string> split(const std::string& str, char delimiter);

// Read entire file content into string
bool read_file(const std::string& path, std::string& content);

// Write string content into file safely
bool write_file(const std::string& path, const std::string& content);

// Creates directory recursively (equivalent to mkdir -p)
bool make_directory_recursive(const std::string& path);

// Formats byte numbers to human-readable string (e.g. 64.0 MB, 1.2 GB)
std::string format_bytes(uint64_t bytes);

// Resolves signal number into standard POSIX name (e.g., SIGKILL, SIGSEGV, SIGSYS)
std::string signal_to_string(int signum);

// Generates random UUID / unique identifier for sandbox instance
std::string generate_uuid();

// Gets current timestamp in milliseconds
uint64_t current_time_millis();

} // namespace utils
} // namespace sandboxx

#endif // SANDBOXX_UTILS_HPP
