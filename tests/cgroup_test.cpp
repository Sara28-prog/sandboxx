#include "sandboxx/cgroup_manager.hpp"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "[TEST] Running Cgroup Manager Test...\n";

    sandboxx::ResourceLimits limits;
    limits.cpu_quota_pct = 25;
    limits.memory_limit_bytes = 64 * 1024 * 1024;
    limits.max_pids = 16;

    sandboxx::CgroupManager mgr("test-cg-01", limits);
    // Path validation
    assert(mgr.get_cgroup_path().find("test-cg-01") != std::string::npos);

    std::cout << "[PASS] Cgroup Manager limits and hierarchy path initialized.\n";
    return 0;
}
