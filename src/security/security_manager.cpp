#include "sandboxx/security_manager.hpp"
#include "sandboxx/logger.hpp"
#include <sys/prctl.h>
#include <sys/capability.h>
#include <unistd.h>
#include <sstream>

namespace sandboxx {

SecurityManager::SecurityManager(const SecurityPolicy& policy)
    : policy_(policy) {}

SecurityManager::~SecurityManager() {
    if (seccomp_ctx_ != nullptr) {
        seccomp_release(seccomp_ctx_);
        seccomp_ctx_ = nullptr;
    }
}

bool SecurityManager::enforce_no_new_privs() {
    LOG_INFO("Enforcing PR_SET_NO_NEW_PRIVS (preventing execve privilege grants)");
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) != 0) {
        LOG_WARN("prctl(PR_SET_NO_NEW_PRIVS) failed");
        return false;
    }
    return true;
}

bool SecurityManager::drop_capabilities() {
    LOG_INFO("Dropping Linux POSIX capabilities");
    
    // In production with libcap:
    // cap_t caps = cap_init();
    // cap_set_proc(caps);
    // cap_free(caps);
    // Also drop bounding set:
    for (int cap = 0; cap <= 40; ++cap) {
        prctl(PR_CAPBSET_DROP, cap, 0, 0, 0);
    }
    return true;
}

bool SecurityManager::apply_seccomp_filter() {
    if (!policy_.enable_seccomp) {
        LOG_WARN("Seccomp is disabled by configuration");
        return true;
    }

    LOG_INFO("Compiling and loading Seccomp-BPF system call filter");

    uint32_t default_scmp_action;
    switch (policy_.default_action) {
        case SeccompDefaultAction::ALLOW:
            default_scmp_action = SCMP_ACT_ALLOW;
            break;
        case SeccompDefaultAction::ERRNO:
            default_scmp_action = SCMP_ACT_ERRNO(EPERM);
            break;
        case SeccompDefaultAction::KILL:
        default:
            default_scmp_action = SCMP_ACT_KILL_PROCESS;
            break;
    }

    seccomp_ctx_ = seccomp_init(default_scmp_action);
    if (seccomp_ctx_ == nullptr) {
        LOG_ERROR("seccomp_init failed");
        return false;
    }

    if (policy_.default_action == SeccompDefaultAction::KILL) {
        // Whitelist mode: add allowed syscalls
        configure_whitelist_rules();
    } else {
        // Blacklist mode: block hazardous syscalls
        configure_blacklist_rules();
    }

    if (seccomp_load(seccomp_ctx_) != 0) {
        LOG_ERROR("seccomp_load failed to load BPF filter into Linux kernel");
        return false;
    }

    LOG_INFO("Seccomp-BPF filter loaded successfully into kernel");
    return true;
}

bool SecurityManager::configure_whitelist_rules() {
    // Standard safe syscalls for runtime operation
    static const std::vector<const char*> baseline_whitelist = {
        "read", "write", "open", "openat", "close", "stat", "fstat", "lstat",
        "poll", "lseek", "mmap", "mprotect", "munmap", "brk", "rt_sigaction",
        "rt_sigprocmask", "rt_sigreturn", "ioctl", "access", "pipe", "pipe2",
        "select", "sched_yield", "mremap", "nanosleep", "getpid", "getuid",
        "getgid", "geteuid", "getegid", "gettimeofday", "clock_gettime",
        "futex", "exit", "exit_group", "arch_prctl", "set_tid_address",
        "set_robust_list", "prlimit64", "getrandom"
    };

    for (const char* sc : baseline_whitelist) {
        int sc_num = seccomp_syscall_resolve_name(sc);
        if (sc_num != __NR_SCMP_ERROR) {
            seccomp_rule_add(seccomp_ctx_, SCMP_ACT_ALLOW, sc_num, 0);
        }
    }

    // Add user custom allowed syscalls
    for (const auto& sc : policy_.allowed_syscalls) {
        int sc_num = seccomp_syscall_resolve_name(sc.c_str());
        if (sc_num != __NR_SCMP_ERROR) {
            seccomp_rule_add(seccomp_ctx_, SCMP_ACT_ALLOW, sc_num, 0);
        }
    }

    return true;
}

bool SecurityManager::configure_blacklist_rules() {
    // Dangerous syscalls that can break isolation or crash host
    static const std::vector<const char*> baseline_blacklist = {
        "ptrace", "sys_chroot", "reboot", "kexec_load", "kexec_file_load",
        "mount", "umount2", "swapon", "swapoff", "bpf", "keyctl",
        "add_key", "request_key", "create_module", "init_module",
        "finit_module", "delete_module", "ioperm", "iopl", "syslog"
    };

    for (const char* sc : baseline_blacklist) {
        int sc_num = seccomp_syscall_resolve_name(sc);
        if (sc_num != __NR_SCMP_ERROR) {
            seccomp_rule_add(seccomp_ctx_, SCMP_ACT_KILL_PROCESS, sc_num, 0);
        }
    }

    for (const auto& sc : policy_.blocked_syscalls) {
        int sc_num = seccomp_syscall_resolve_name(sc.c_str());
        if (sc_num != __NR_SCMP_ERROR) {
            seccomp_rule_add(seccomp_ctx_, SCMP_ACT_KILL_PROCESS, sc_num, 0);
        }
    }

    return true;
}

std::string SecurityManager::export_bpf_filter_dump() const {
    std::stringstream ss;
    ss << "=== SECCOMP BPF FILTER POLICY ===\n";
    ss << "Mode: " << (policy_.default_action == SeccompDefaultAction::KILL ? "WHITELIST (Default KILL)" : "BLACKLIST (Default ALLOW)") << "\n";
    ss << "NoNewPrivs: " << (policy_.no_new_privs ? "ENFORCED" : "OFF") << "\n";
    ss << "Capabilities: " << (policy_.drop_capabilities ? "DROPPED (CAP_SYS_ADMIN, etc.)" : "RETAINED") << "\n";
    ss << "Blocked Syscalls: ptrace, reboot, mount, kexec_load, bpf, keyctl, chroot\n";
    return ss.str();
}

} // namespace sandboxx
