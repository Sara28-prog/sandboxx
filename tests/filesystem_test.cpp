#include "sandboxx/filesystem_manager.hpp"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "[TEST] Running Filesystem Manager Test...\n";

    sandboxx::SandboxConfig cfg = sandboxx::SandboxConfig::create_default();
    cfg.rootfs_path = "./rootfs";
    
    sandboxx::FilesystemManager fs_mgr(cfg);
    std::cout << "[PASS] Filesystem jail initialized with rootfs: " << cfg.rootfs_path << "\n";
    return 0;
}
