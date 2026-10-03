#ifndef SANDBOXX_RUNTIME_HPP
#define SANDBOXX_RUNTIME_HPP

#include "sandbox_config.hpp"
#include <string>
#include <memory>
#include <vector>
#include <chrono>

namespace sandboxx {

enum class SandboxState {
    CREATED,
    INITIALIZING,
    RUNNING,
    STOPPED,
    KILLED_OOM,
    KILLED_SECCOMP,
    KILLED_TIMEOUT,
    EXITED
};

struct ExecutionStats {
    pid_t host_pid{-1};
    int exit_code{-1};
    int exit_signal{0};
    uint64_t peak_memory_bytes{0};
    double cpu_user_time_sec{0.0};
    double cpu_sys_time_sec{0.0};
    uint64_t total_syscalls{0};
    std::chrono::milliseconds wall_clock_time{0};
    bool oom_killed{false};
    bool seccomp_violation{false};
    bool timeout_reached{false};
};

class ProcessManager;
class NamespaceManager;
class CgroupManager;
class FilesystemManager;
class SecurityManager;
class Monitor;

class SandboxInstance {
public:
    explicit SandboxInstance(const SandboxConfig& config);
    ~SandboxInstance();

    bool initialize();
    bool start();
    bool wait();
    bool stop(int signal = 15);
    bool kill();

    SandboxState get_state() const { return state_; }
    const SandboxConfig& get_config() const { return config_; }
    const ExecutionStats& get_stats() const { return stats_; }
    pid_t get_host_pid() const;

private:
    SandboxConfig config_;
    SandboxState state_{SandboxState::CREATED};
    ExecutionStats stats_;
    
    std::unique_ptr<ProcessManager> process_mgr_;
    std::unique_ptr<NamespaceManager> namespace_mgr_;
    std::unique_ptr<CgroupManager> cgroup_mgr_;
    std::unique_ptr<FilesystemManager> fs_mgr_;
    std::unique_ptr<SecurityManager> security_mgr_;
    std::unique_ptr<Monitor> monitor_;
};

class Runtime {
public:
    static Runtime& get_instance();
    
    std::string create_sandbox(const SandboxConfig& config);
    std::shared_ptr<SandboxInstance> get_sandbox(const std::string& id);
    std::vector<std::shared_ptr<SandboxInstance>> list_sandboxes();
    bool remove_sandbox(const std::string& id);

private:
    Runtime() = default;
    std::map<std::string, std::shared_ptr<SandboxInstance>> active_sandboxes_;
};

} // namespace sandboxx

#endif // SANDBOXX_RUNTIME_HPP
