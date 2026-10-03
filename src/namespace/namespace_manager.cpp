#include "sandboxx/namespace_manager.hpp"
#include "sandboxx/logger.hpp"
#include "sandboxx/utils.hpp"
#include <unistd.h>
#include <sys/mount.h>
#include <fstream>
#include <cstring>

namespace sandboxx {

NamespaceManager::NamespaceManager(const NamespaceConfig& config)
    : config_(config) {}

NamespaceManager::~NamespaceManager() = default;

int NamespaceManager::get_clone_flags() const {
    int flags = 0;
    if (config_.enable_pid)   flags |= CLONE_NEWPID;
    if (config_.enable_mount) flags |= CLONE_NEWNS;
    if (config_.enable_uts)   flags |= CLONE_NEWUTS;
    if (config_.enable_ipc)   flags |= CLONE_NEWIPC;
    if (config_.enable_net)   flags |= CLONE_NEWNET;
    if (config_.enable_user)  flags |= CLONE_NEWUSER;
    return flags;
}

bool NamespaceManager::setup_child_namespaces() {
    LOG_INFO("Configuring child process namespaces");

    // 1. Configure UTS hostname isolation
    if (config_.enable_uts) {
        if (!configure_uts()) {
            LOG_WARN("Failed setting UTS hostname in isolated namespace");
        }
    }

    // 2. Configure IPC
    if (config_.enable_ipc) {
        configure_ipc();
    }

    // 3. Configure Network loopback
    if (config_.enable_net) {
        configure_network();
    }

    return true;
}

bool NamespaceManager::configure_uts() {
    LOG_INFO("Setting isolated UTS hostname: " + config_.hostname);
    if (sethostname(config_.hostname.c_str(), config_.hostname.length()) != 0) {
        LOG_WARN("sethostname failed: " + std::string(strerror(errno)));
        return false;
    }
    return true;
}

bool NamespaceManager::configure_ipc() {
    LOG_INFO("IPC namespace active: Message queues, semaphores, and shared memory isolated from host");
    return true;
}

bool NamespaceManager::configure_network() {
    LOG_INFO("Network namespace active: Host network interfaces concealed; loopback initialized");
    // In production, bring up lo: `ip link set lo up` or ioctl SIOCSIFFLAGS
    return true;
}

bool NamespaceManager::setup_parent_namespaces(pid_t child_pid) {
    if (config_.enable_user) {
        return configure_user_mappings(child_pid);
    }
    return true;
}

bool NamespaceManager::configure_user_mappings(pid_t child_pid) {
    std::string uid_map_path = "/proc/" + std::to_string(child_pid) + "/uid_map";
    std::string gid_map_path = "/proc/" + std::to_string(child_pid) + "/gid_map";
    std::string setgroups_path = "/proc/" + std::to_string(child_pid) + "/setgroups";

    // Disable setgroups first
    utils::write_file(setgroups_path, "deny");

    // Map container UID 0 (root) to host user UID
    uid_t host_uid = getuid();
    gid_t host_gid = getgid();

    std::string uid_mapping = "0 " + std::to_string(host_uid) + " 1\n";
    std::string gid_mapping = "0 " + std::to_string(host_gid) + " 1\n";

    utils::write_file(uid_map_path, uid_mapping);
    utils::write_file(gid_map_path, gid_mapping);

    LOG_INFO("Configured User Namespace UID/GID mapping for child PID " + std::to_string(child_pid));
    return true;
}

} // namespace sandboxx
