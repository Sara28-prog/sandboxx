#include "sandboxx/runtime.hpp"
#include "sandboxx/process_manager.hpp"
#include "sandboxx/namespace_manager.hpp"
#include "sandboxx/cgroup_manager.hpp"
#include "sandboxx/filesystem_manager.hpp"
#include "sandboxx/security_manager.hpp"
#include "sandboxx/monitor.hpp"
#include "sandboxx/logger.hpp"
#include "sandboxx/utils.hpp"
#include <unistd.h>
#include <sstream>
#include <cstring>

namespace sandboxx {

// Factory methods for configs
SandboxConfig SandboxConfig::create_default() {
    SandboxConfig cfg;
    cfg.id = "sbx-" + utils::generate_uuid();
    cfg.name = "default-profile";
    cfg.rootfs_path = "./rootfs";
    cfg.working_dir = "/";
    cfg.resources.cpu_quota_pct = 50;
    cfg.resources.memory_limit_bytes = 128 * 1024 * 1024;
    cfg.resources.max_pids = 32;
    cfg.resources.execution_timeout_sec = 30;
    cfg.namespaces.enable_pid = true;
    cfg.namespaces.enable_mount = true;
    cfg.namespaces.enable_uts = true;
    cfg.namespaces.enable_ipc = true;
    cfg.namespaces.enable_net = true;
    cfg.namespaces.hostname = "sandboxx-jail";
    cfg.security.enable_seccomp = true;
    cfg.security.default_action = SeccompDefaultAction::KILL;
    cfg.security.no_new_privs = true;
    cfg.security.drop_capabilities = true;
    return cfg;
}

SandboxConfig SandboxConfig::create_restricted() {
    SandboxConfig cfg = create_default();
    cfg.name = "restricted-profile";
    cfg.resources.cpu_quota_pct = 20;
    cfg.resources.memory_limit_bytes = 64 * 1024 * 1024; // 64 MB
    cfg.resources.max_pids = 16;
    cfg.resources.execution_timeout_sec = 10;
    cfg.security.default_action = SeccompDefaultAction::KILL;
    return cfg;
}

SandboxConfig SandboxConfig::create_development() {
    SandboxConfig cfg = create_default();
    cfg.name = "development-profile";
    cfg.resources.cpu_quota_pct = 100;
    cfg.resources.memory_limit_bytes = 512 * 1024 * 1024; // 512 MB
    cfg.resources.max_pids = 128;
    cfg.resources.execution_timeout_sec = 120;
    cfg.security.default_action = SeccompDefaultAction::ALLOW;
    return cfg;
}

SandboxConfig SandboxConfig::load_from_json_file(const std::string& filepath) {
    SandboxConfig cfg = create_default();
    std::string content;
    if (!utils::read_file(filepath, content)) {
        LOG_WARN("Could not read config file " + filepath + "; using default config");
        return cfg;
    }

    // Basic JSON parser for sandbox configuration
    if (content.find("\"cpu_quota_pct\"") != std::string::npos) {
        size_t pos = content.find("\"cpu_quota_pct\"");
        size_t colon = content.find(':', pos);
        if (colon != std::string::npos) {
            cfg.resources.cpu_quota_pct = std::stoi(content.substr(colon + 1));
        }
    }
    if (content.find("\"memory_limit_mb\"") != std::string::npos) {
        size_t pos = content.find("\"memory_limit_mb\"");
        size_t colon = content.find(':', pos);
        if (colon != std::string::npos) {
            uint64_t mb = std::stoull(content.substr(colon + 1));
            cfg.resources.memory_limit_bytes = mb * 1024 * 1024;
        }
    }
    if (content.find("\"max_pids\"") != std::string::npos) {
        size_t pos = content.find("\"max_pids\"");
        size_t colon = content.find(':', pos);
        if (colon != std::string::npos) {
            cfg.resources.max_pids = std::stoi(content.substr(colon + 1));
        }
    }
    if (content.find("\"timeout_sec\"") != std::string::npos) {
        size_t pos = content.find("\"timeout_sec\"");
        size_t colon = content.find(':', pos);
        if (colon != std::string::npos) {
            cfg.resources.execution_timeout_sec = std::stoi(content.substr(colon + 1));
        }
    }
    if (content.find("\"rootfs_path\"") != std::string::npos) {
        size_t pos = content.find("\"rootfs_path\"");
        size_t quote1 = content.find('"', pos + 14);
        size_t quote2 = content.find('"', quote1 + 1);
        if (quote1 != std::string::npos && quote2 != std::string::npos) {
            cfg.rootfs_path = content.substr(quote1 + 1, quote2 - quote1 - 1);
        }
    }
    return cfg;
}

std::string SandboxConfig::to_json_string() const {
    std::stringstream ss;
    ss << "{\n"
       << "  \"id\": \"" << id << "\",\n"
       << "  \"name\": \"" << name << "\",\n"
       << "  \"rootfs_path\": \"" << rootfs_path << "\",\n"
       << "  \"working_dir\": \"" << working_dir << "\",\n"
       << "  \"binary_path\": \"" << binary_path << "\",\n"
       << "  \"resources\": {\n"
       << "    \"cpu_quota_pct\": " << resources.cpu_quota_pct << ",\n"
       << "    \"memory_limit_bytes\": " << resources.memory_limit_bytes << ",\n"
       << "    \"max_pids\": " << resources.max_pids << ",\n"
       << "    \"timeout_sec\": " << resources.execution_timeout_sec << "\n"
       << "  },\n"
       << "  \"namespaces\": {\n"
       << "    \"pid\": " << (namespaces.enable_pid ? "true" : "false") << ",\n"
       << "    \"mount\": " << (namespaces.enable_mount ? "true" : "false") << ",\n"
       << "    \"uts\": " << (namespaces.enable_uts ? "true" : "false") << ",\n"
       << "    \"ipc\": " << (namespaces.enable_ipc ? "true" : "false") << ",\n"
       << "    \"net\": " << (namespaces.enable_net ? "true" : "false") << ",\n"
       << "    \"hostname\": \"" << namespaces.hostname << "\"\n"
       << "  },\n"
       << "  \"security\": {\n"
       << "    \"seccomp\": " << (security.enable_seccomp ? "true" : "false") << ",\n"
       << "    \"no_new_privs\": " << (security.no_new_privs ? "true" : "false") << ",\n"
       << "    \"drop_caps\": " << (security.drop_capabilities ? "true" : "false") << "\n"
       << "  }\n"
       << "}";
    return ss.str();
}

// SandboxInstance implementation
SandboxInstance::SandboxInstance(const SandboxConfig& config)
    : config_(config) {
    process_mgr_ = std::make_unique<ProcessManager>(config_);
    namespace_mgr_ = std::make_unique<NamespaceManager>(config_.namespaces);
    cgroup_mgr_ = std::make_unique<CgroupManager>(config_.id, config_.resources);
    fs_mgr_ = std::make_unique<FilesystemManager>(config_);
    security_mgr_ = std::make_unique<SecurityManager>(config_.security);
}

SandboxInstance::~SandboxInstance() {
    stop();
}

bool SandboxInstance::initialize() {
    state_ = SandboxState::INITIALIZING;
    LOG_INFO("Initializing sandbox instance [" + config_.id + "]");

    // Initialize Cgroup v2
    cgroup_mgr_->initialize();
    cgroup_mgr_->apply_limits();

    return true;
}

bool SandboxInstance::start() {
    if (state_ != SandboxState::INITIALIZING) {
        initialize();
    }

    LOG_INFO("Starting sandbox execution for target: " + config_.binary_path);

    int clone_flags = namespace_mgr_->get_clone_flags();

    // Spawn child
    pid_t child_pid = process_mgr_->spawn([this]() -> int {
        // Child execution path
        LOG_INFO("[Child] Configuring filesystem mounts & isolation");
        fs_mgr_->setup_mount_namespace();
        fs_mgr_->isolate_rootfs();
        fs_mgr_->mount_essential_filesystems();
        fs_mgr_->mount_custom_entries();
        fs_mgr_->mask_sensitive_paths();
        fs_mgr_->enter_working_dir();

        LOG_INFO("[Child] Configuring isolated namespaces");
        namespace_mgr_->setup_child_namespaces();

        LOG_INFO("[Child] Enforcing security policies (NoNewPrivs, Caps, Seccomp)");
        security_mgr_->enforce_no_new_privs();
        security_mgr_->drop_capabilities();
        security_mgr_->apply_seccomp_filter();

        // Convert arguments to C-style argv
        std::vector<char*> argv_ptrs;
        argv_ptrs.push_back(const_cast<char*>(config_.binary_path.c_str()));
        for (const auto& arg : config_.args) {
            argv_ptrs.push_back(const_cast<char*>(arg.c_str()));
        }
        argv_ptrs.push_back(nullptr);

        LOG_INFO("[Child] Calling execve for: " + config_.binary_path);
        execv(config_.binary_path.c_str(), argv_ptrs.data());

        // If execv returns, an error occurred
        std::cerr << "SandBoxX Child execv failed: " << strerror(errno) << std::endl;
        return 127;
    }, clone_flags);

    if (child_pid <= 0) {
        LOG_ERROR("Failed to spawn sandboxed process");
        state_ = SandboxState::STOPPED;
        return false;
    }

    stats_.host_pid = child_pid;

    // Parent side configuration: attach child to cgroup & setup user mapping
    cgroup_mgr_->attach_process(child_pid);
    namespace_mgr_->setup_parent_namespaces(child_pid);

    // Unblock child execution
    process_mgr_->notify_child_ready();

    // Start telemetry monitor
    monitor_ = std::make_unique<Monitor>(child_pid, cgroup_mgr_.get());
    monitor_->start();

    state_ = SandboxState::RUNNING;
    return true;
}

bool SandboxInstance::wait() {
    if (state_ != SandboxState::RUNNING) return false;

    auto start_time = std::chrono::steady_clock::now();

    int exit_sig = 0;
    bool oom = false;
    int exit_code = process_mgr_->wait_for_exit(&exit_sig, &oom);

    auto end_time = std::chrono::steady_clock::now();
    stats_.wall_clock_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    stats_.exit_code = exit_code;
    stats_.exit_signal = exit_sig;

    // Stop monitor
    if (monitor_) {
        monitor_->stop();
        MetricSnapshot last_metric = monitor_->get_latest_metrics();
        stats_.peak_memory_bytes = last_metric.memory_rss_bytes;
    }

    // Check Cgroup for OOM events
    if (cgroup_mgr_ && (oom || cgroup_mgr_->has_oom_occurred())) {
        stats_.oom_killed = true;
        state_ = SandboxState::KILLED_OOM;
        LOG_WARN("Sandbox [" + config_.id + "] was KILLED BY OOM (Memory limit exceeded)");
    } else if (exit_sig == 31 /* SIGSYS */) {
        stats_.seccomp_violation = true;
        state_ = SandboxState::KILLED_SECCOMP;
        LOG_WARN("Sandbox [" + config_.id + "] was KILLED BY SECCOMP (Syscall violation)");
    } else {
        state_ = SandboxState::EXITED;
        LOG_INFO("Sandbox [" + config_.id + "] exited with code " + std::to_string(exit_code));
    }

    return true;
}

bool SandboxInstance::stop(int signal) {
    if (process_mgr_) {
        return process_mgr_->terminate(signal);
    }
    return false;
}

bool SandboxInstance::kill() {
    return stop(9); // SIGKILL
}

pid_t SandboxInstance::get_host_pid() const {
    return stats_.host_pid;
}

// Runtime singleton
Runtime& Runtime::get_instance() {
    static Runtime instance;
    return instance;
}

std::string Runtime::create_sandbox(const SandboxConfig& config) {
    auto instance = std::make_shared<SandboxInstance>(config);
    active_sandboxes_[config.id] = instance;
    return config.id;
}

std::shared_ptr<SandboxInstance> Runtime::get_sandbox(const std::string& id) {
    auto it = active_sandboxes_.find(id);
    if (it != active_sandboxes_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<SandboxInstance>> Runtime::list_sandboxes() {
    std::vector<std::shared_ptr<SandboxInstance>> list;
    for (const auto& pair : active_sandboxes_) {
        list.push_back(pair.second);
    }
    return list;
}

bool Runtime::remove_sandbox(const std::string& id) {
    auto it = active_sandboxes_.find(id);
    if (it != active_sandboxes_.end()) {
        it->second->stop(9);
        active_sandboxes_.erase(it);
        return true;
    }
    return false;
}

} // namespace sandboxx
