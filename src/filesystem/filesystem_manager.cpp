#include "sandboxx/filesystem_manager.hpp"
#include "sandboxx/logger.hpp"
#include "sandboxx/utils.hpp"
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <cstring>

namespace sandboxx {

FilesystemManager::FilesystemManager(const SandboxConfig& config)
    : config_(config), rootfs_dir_(config.rootfs_path) {
    old_root_dir_ = rootfs_dir_ + "/.old_root";
}

FilesystemManager::~FilesystemManager() = default;

bool FilesystemManager::mount_bind(const std::string& src, const std::string& dst, bool readonly) {
    utils::make_directory_recursive(dst);
    unsigned long flags = MS_BIND | MS_REC;
    if (mount(src.c_str(), dst.c_str(), nullptr, flags, nullptr) != 0) {
        LOG_WARN("Bind mount failed from " + src + " to " + dst + ": " + strerror(errno));
        return false;
    }

    if (readonly) {
        flags |= MS_REMOUNT | MS_RDONLY;
        if (mount(nullptr, dst.c_str(), nullptr, flags, nullptr) != 0) {
            LOG_WARN("Remount read-only failed for " + dst + ": " + strerror(errno));
            return false;
        }
    }
    return true;
}

bool FilesystemManager::mount_pseudo(const std::string& fstype, const std::string& target, unsigned long flags) {
    utils::make_directory_recursive(target);
    if (mount(fstype.c_str(), target.c_str(), fstype.c_str(), flags, nullptr) != 0) {
        LOG_WARN("Failed mounting " + fstype + " on " + target + ": " + strerror(errno));
        return false;
    }
    return true;
}

bool FilesystemManager::setup_mount_namespace() {
    LOG_INFO("Configuring private mount propagation");

    // Make all mounts private to avoid leaking mount events to host
    if (mount(nullptr, "/", nullptr, MS_REC | MS_PRIVATE, nullptr) != 0) {
        LOG_WARN("Failed to make mounts private: " + std::string(strerror(errno)));
    }

    return true;
}

bool FilesystemManager::isolate_rootfs() {
    LOG_INFO("Isolating root filesystem into: " + rootfs_dir_);

    // Check if rootfs directory exists
    struct stat st;
    if (stat(rootfs_dir_.c_str(), &st) != 0) {
        LOG_WARN("Rootfs path [" + rootfs_dir_ + "] does not exist; running in restricted chroot fallback mode");
        return false;
    }

    // Ensure rootfs is a mount point for pivot_root
    mount(rootfs_dir_.c_str(), rootfs_dir_.c_str(), "bind", MS_BIND | MS_REC, nullptr);

    // Create old root mount directory for pivot_root
    utils::make_directory_recursive(old_root_dir_);

    // Attempt pivot_root (syscall 155 on x86_64)
    if (syscall(SYS_pivot_root, rootfs_dir_.c_str(), old_root_dir_.c_str()) == 0) {
        LOG_INFO("pivot_root succeeded; root shifted to " + rootfs_dir_);
        chdir("/");
        // Unmount old root
        if (umount2("/.old_root", MNT_DETACH) != 0) {
            LOG_WARN("Failed unmounting old root: " + std::string(strerror(errno)));
        } else {
            rmdir("/.old_root");
        }
    } else {
        LOG_WARN("pivot_root failed (" + std::string(strerror(errno)) + "); falling back to chroot");
        if (chroot(rootfs_dir_.c_str()) != 0) {
            LOG_ERROR("chroot failed: " + std::string(strerror(errno)));
            return false;
        }
        chdir("/");
    }

    return true;
}

bool FilesystemManager::mount_essential_filesystems() {
    LOG_INFO("Mounting container essential filesystems: /proc, /sys (RO), /tmp (tmpfs)");

    // Mount clean /proc
    mount_pseudo("proc", "/proc", MS_NOSUID | MS_NODEV | MS_NOEXEC);

    // Mount /sys as read-only
    mount_pseudo("sysfs", "/sys", MS_NOSUID | MS_NODEV | MS_NOEXEC | MS_RDONLY);

    // Mount /tmp as isolated tmpfs
    mount_pseudo("tmpfs", "/tmp", MS_NOSUID | MS_NODEV);

    return true;
}

bool FilesystemManager::mount_custom_entries() {
    for (const auto& entry : config_.custom_mounts) {
        LOG_INFO("Mounting custom entry: " + entry.source + " -> " + entry.target + 
                 (entry.is_readonly ? " [ro]" : " [rw]"));
        mount_bind(entry.source, entry.target, entry.is_readonly);
    }
    return true;
}

bool FilesystemManager::mask_sensitive_paths() {
    // Mask sensitive kernel files by mounting read-only tmpfs or empty node over them
    static const std::vector<std::string> masked_paths = {
        "/proc/kcore",
        "/proc/sysrq-trigger",
        "/proc/irq",
        "/proc/bus",
        "/sys/firmware"
    };

    for (const auto& path : masked_paths) {
        struct stat st;
        if (stat(path.c_str(), &st) == 0) {
            mount("/dev/null", path.c_str(), nullptr, MS_BIND, nullptr);
            mount(nullptr, path.c_str(), nullptr, MS_BIND | MS_REMOUNT | MS_RDONLY, nullptr);
        }
    }

    return true;
}

bool FilesystemManager::enter_working_dir() {
    const std::string& dir = config_.working_dir.empty() ? "/" : config_.working_dir;
    LOG_INFO("Entering working directory: " + dir);
    return chdir(dir.c_str()) == 0;
}

} // namespace sandboxx
