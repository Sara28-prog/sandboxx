#ifndef SANDBOXX_CONFIG_HPP
#define SANDBOXX_CONFIG_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <map>

namespace sandboxx {

enum class SeccompDefaultAction {
    ALLOW,
    KILL,
    ERRNO
};

struct ResourceLimits {
    uint32_t cpu_quota_pct{50};      // e.g., 50% CPU allocation
    uint64_t memory_limit_bytes{128 * 1024 * 1024}; // 128 MB default
    uint32_t max_pids{32};            // max processes/threads
    uint64_t disk_read_bps{10 * 1024 * 1024}; // 10 MB/s
    uint64_t disk_write_bps{10 * 1024 * 1024};
    uint32_t execution_timeout_sec{30};
};

struct MountEntry {
    std::string source;
    std::string target;
    std::string fstype;
    unsigned long flags{0};
    bool is_readonly{true};
};

struct NamespaceConfig {
    bool enable_pid{true};
    bool enable_mount{true};
    bool enable_uts{true};
    bool enable_ipc{true};
    bool enable_net{true};
    bool enable_user{false}; // Requires subuid/subgid mapping
    std::string hostname{"sandboxx-jail"};
};

struct SecurityPolicy {
    bool enable_seccomp{true};
    SeccompDefaultAction default_action{SeccompDefaultAction::KILL};
    std::vector<std::string> allowed_syscalls;
    std::vector<std::string> blocked_syscalls;
    bool drop_capabilities{true};
    bool no_new_privs{true};
};

struct SandboxConfig {
    std::string id;
    std::string name;
    std::string rootfs_path{"./rootfs"};
    std::string working_dir{"/"};
    std::string binary_path;
    std::vector<std::string> args;
    std::map<std::string, std::string> env;
    
    ResourceLimits resources;
    NamespaceConfig namespaces;
    SecurityPolicy security;
    std::vector<MountEntry> custom_mounts;

    static SandboxConfig load_from_json_file(const std::string& filepath);
    static SandboxConfig create_default();
    static SandboxConfig create_restricted();
    static SandboxConfig create_development();
    std::string to_json_string() const;
};

} // namespace sandboxx

#endif // SANDBOXX_CONFIG_HPP
