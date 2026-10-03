#include <cstring>
#include "sandboxx/utils.hpp"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>
#include <csignal>
#include <chrono>

namespace sandboxx {
namespace utils {

bool is_root_user() {
    return (geteuid() == 0);
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream token_stream(str);
    while (std::getline(token_stream, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

bool read_file(const std::string& path, std::string& content) {
    std::ifstream file(path);
    if (!file.is_open()) return false;
    std::stringstream buffer;
    buffer << file.rdbuf();
    content = buffer.str();
    return true;
}

bool write_file(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) return false;
    file << content;
    return file.good();
}

bool make_directory_recursive(const std::string& path) {
    char tmp[512];
    snprintf(tmp, sizeof(tmp), "%s", path.c_str());
    size_t len = strlen(tmp);
    if (len == 0) return false;

    if (tmp[len - 1] == '/') tmp[len - 1] = 0;

    for (char* p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
            *p = '/';
        }
    }
    return mkdir(tmp, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0 || errno == EEXIST;
}

std::string format_bytes(uint64_t bytes) {
    static const char* suffixes[] = {"B", "KB", "MB", "GB", "TB"};
    int i = 0;
    double d_bytes = static_cast<double>(bytes);
    while (d_bytes >= 1024.0 && i < 4) {
        d_bytes /= 1024.0;
        i++;
    }
    char buf[64];
    snprintf(buf, sizeof(buf), "%.2f %s", d_bytes, suffixes[i]);
    return std::string(buf);
}

std::string signal_to_string(int signum) {
    switch (signum) {
        case SIGTERM: return "SIGTERM (Termination)";
        case SIGKILL: return "SIGKILL (Killed by kernel/OOM)";
        case SIGSEGV: return "SIGSEGV (Segmentation Fault)";
        case SIGSYS:  return "SIGSYS (Seccomp Bad System Call)";
        case SIGABRT: return "SIGABRT (Aborted)";
        case SIGALRM: return "SIGALRM (Timeout Alarm)";
        case SIGFPE:  return "SIGFPE (Floating Point Exception)";
        case SIGILL:  return "SIGILL (Illegal Instruction)";
        case SIGINT:  return "SIGINT (Interrupt)";
        default: return "Signal " + std::to_string(signum);
    }
}

std::string generate_uuid() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint32_t> dis(0, 0xFFFFFFFF);

    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(8) << dis(gen);
    return ss.str();
}

uint64_t current_time_millis() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

} // namespace utils
} // namespace sandboxx
