#include "sandboxx/cgroup_manager.hpp"
#include "sandboxx/logger.hpp"
#include "sandboxx/utils.hpp"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

namespace sandboxx {

CgroupManager::CgroupManager(const std::string& sandbox_id, const ResourceLimits& limits)
    : sandbox_id_(sandbox_id), limits_(limits) {
    cgroup_path_ = cgroup_base_path_ + "/" + sandbox_id_;
}

CgroupManager::~CgroupManager() {
    cleanup();
}

bool CgroupManager::write_cgroup_file(const std::string& filename, const std::string& value) {
    std::string full_path = cgroup_path_ + "/" + filename;
    if (!utils::write_file(full_path, value)) {
        LOG_WARN("Failed writing " + value + " to " + full_path + " (may require root or cgroup v2 delegation)");
        return false;
    }
    return true;
}

std::string CgroupManager::read_cgroup_file(const std::string& filename) {
    std::string full_path = cgroup_path_ + "/" + filename;
    std::string content;
    utils::read_file(full_path, content);
    return content;
}

bool CgroupManager::initialize() {
    LOG_INFO("Initializing Cgroup v2 for sandbox [" + sandbox_id_ + "] at " + cgroup_path_);
    
    // Check if /sys/fs/cgroup exists
    struct stat st;
    if (stat("/sys/fs/cgroup", &st) != 0) {
        LOG_WARN("/sys/fs/cgroup not accessible; running in simulation/fallback mode");
        return false;
    }

    // Ensure base directory exists
    utils::make_directory_recursive(cgroup_base_path_);

    // Enable subtree_control in parent if available
    std::string parent_subtree = cgroup_base_path_ + "/cgroup.subtree_control";
    utils::write_file(parent_subtree, "+cpu +memory +pids +io");

    // Create sandbox cgroup folder
    if (!utils::make_directory_recursive(cgroup_path_)) {
        LOG_WARN("Cannot create cgroup directory: " + cgroup_path_);
        return false;
    }

    return true;
}

bool CgroupManager::attach_process(pid_t pid) {
    LOG_INFO("Attaching PID " + std::to_string(pid) + " to cgroup " + cgroup_path_);
    return write_cgroup_file("cgroup.procs", std::to_string(pid));
}

bool CgroupManager::apply_limits() {
    LOG_INFO("Enforcing resource limits: CPU Quota=" + std::to_string(limits_.cpu_quota_pct) + 
             "%, Memory=" + utils::format_bytes(limits_.memory_limit_bytes) + 
             ", Max PIDs=" + std::to_string(limits_.max_pids));

    bool success = true;

    // 1. CPU Quota via cpu.max: "quota period" (e.g. 50000 100000 for 50%)
    uint64_t period = 100000; // 100ms standard period
    uint64_t quota = (period * limits_.cpu_quota_pct) / 100;
    std::string cpu_val = std::to_string(quota) + " " + std::to_string(period);
    if (!write_cgroup_file("cpu.max", cpu_val)) {
        success = false;
    }

    // 2. Memory limit via memory.max
    if (!write_cgroup_file("memory.max", std::to_string(limits_.memory_limit_bytes))) {
        success = false;
    }

    // Set memory.high at 90% of max for proactive throttling before OOM
    uint64_t memory_high = (limits_.memory_limit_bytes * 9) / 10;
    write_cgroup_file("memory.high", std::to_string(memory_high));

    // Enable memory.oom.group so entire process tree is cleanly terminated on OOM
    write_cgroup_file("memory.oom.group", "1");

    // 3. Process count via pids.max
    if (!write_cgroup_file("pids.max", std::to_string(limits_.max_pids))) {
        success = false;
    }

    return success;
}

CgroupStats CgroupManager::read_stats() {
    CgroupStats stats;

    // Read current memory usage
    std::string mem_str = read_cgroup_file("memory.current");
    if (!mem_str.empty()) {
        try {
            stats.memory_current_bytes = std::stoull(mem_str);
        } catch (...) {}
    }

    // Read peak memory usage
    std::string mem_peak_str = read_cgroup_file("memory.peak");
    if (!mem_peak_str.empty()) {
        try {
            stats.memory_peak_bytes = std::stoull(mem_peak_str);
        } catch (...) {}
    }

    // Read memory events (oom kill count)
    std::string mem_events = read_cgroup_file("memory.events");
    if (!mem_events.empty()) {
        std::istringstream stream(mem_events);
        std::string key;
        uint64_t val;
        while (stream >> key >> val) {
            if (key == "oom_kill") {
                stats.oom_kill_count = val;
            }
        }
    }

    // Read CPU stat
    std::string cpu_stat = read_cgroup_file("cpu.stat");
    if (!cpu_stat.empty()) {
        std::istringstream stream(cpu_stat);
        std::string key;
        uint64_t val;
        while (stream >> key >> val) {
            if (key == "usage_usec") stats.cpu_usage_usec = val;
            else if (key == "user_usec") stats.cpu_user_usec = val;
            else if (key == "system_usec") stats.cpu_system_usec = val;
            else if (key == "nr_throttled") stats.nr_throttled = val;
            else if (key == "throttled_usec") stats.throttled_usec = val;
        }
    }

    // Read current PID count
    std::string pids_current = read_cgroup_file("pids.current");
    if (!pids_current.empty()) {
        try {
            stats.current_pids = std::stoul(pids_current);
        } catch (...) {}
    }

    return stats;
}

bool CgroupManager::has_oom_occurred() {
    CgroupStats stats = read_stats();
    return stats.oom_kill_count > 0;
}

bool CgroupManager::cleanup() {
    if (cgroup_path_.empty()) return true;
    LOG_INFO("Removing Cgroup directory: " + cgroup_path_);
    // Remove directory
    return rmdir(cgroup_path_.c_str()) == 0;
}

} // namespace sandboxx
