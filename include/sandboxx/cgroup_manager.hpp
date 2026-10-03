#ifndef SANDBOXX_CGROUP_MANAGER_HPP
#define SANDBOXX_CGROUP_MANAGER_HPP

#include "sandbox_config.hpp"
#include <string>
#include <cstdint>

namespace sandboxx {

struct CgroupStats {
    uint64_t memory_current_bytes{0};
    uint64_t memory_peak_bytes{0};
    uint64_t oom_kill_count{0};
    uint64_t cpu_usage_usec{0};
    uint64_t cpu_user_usec{0};
    uint64_t cpu_system_usec{0};
    uint64_t nr_throttled{0};
    uint64_t throttled_usec{0};
    uint32_t current_pids{0};
};

class CgroupManager {
public:
    CgroupManager(const std::string& sandbox_id, const ResourceLimits& limits);
    ~CgroupManager();

    // Creates the cgroup v2 directory under /sys/fs/cgroup/sandboxx/<id>
    bool initialize();

    // Attaches a target process to cgroup.procs
    bool attach_process(pid_t pid);

    // Configures cpu.max, memory.max, pids.max, io.max
    bool apply_limits();

    // Query real-time metrics from cgroup v2 controllers
    CgroupStats read_stats();

    // Check if the cgroup experienced an Out-Of-Memory termination
    bool has_oom_occurred();

    // Removes the cgroup directory upon termination
    bool cleanup();

    std::string get_cgroup_path() const { return cgroup_path_; }

private:
    std::string sandbox_id_;
    ResourceLimits limits_;
    std::string cgroup_base_path_{"/sys/fs/cgroup/sandboxx"};
    std::string cgroup_path_;
    bool is_v2_{true};

    bool write_cgroup_file(const std::string& filename, const std::string& value);
    std::string read_cgroup_file(const std::string& filename);
};

} // namespace sandboxx

#endif // SANDBOXX_CGROUP_MANAGER_HPP
