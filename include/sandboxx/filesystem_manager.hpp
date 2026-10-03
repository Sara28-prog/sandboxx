#ifndef SANDBOXX_FILESYSTEM_MANAGER_HPP
#define SANDBOXX_FILESYSTEM_MANAGER_HPP

#include "sandbox_config.hpp"
#include <string>
#include <vector>

namespace sandboxx {

class FilesystemManager {
public:
    explicit FilesystemManager(const SandboxConfig& config);
    ~FilesystemManager();

    // Setup private mount propagation and filesystem jail
    bool setup_mount_namespace();

    // Performs pivot_root or chroot into the isolated rootfs
    bool isolate_rootfs();

    // Mounts clean /proc, /sys (read-only), /dev minimal nodes, /tmp (tmpfs)
    bool mount_essential_filesystems();

    // Mounts caller-specified bind directories with ro/rw flags
    bool mount_custom_entries();

    // Masks dangerous kernel interfaces like /proc/sys, /proc/kcore, /sys/firmware
    bool mask_sensitive_paths();

    // Switches working directory inside jailed filesystem
    bool enter_working_dir();

private:
    SandboxConfig config_;
    std::string rootfs_dir_;
    std::string old_root_dir_;

    bool mount_bind(const std::string& src, const std::string& dst, bool readonly);
    bool mount_pseudo(const std::string& fstype, const std::string& target, unsigned long flags);
};

} // namespace sandboxx

#endif // SANDBOXX_FILESYSTEM_MANAGER_HPP
