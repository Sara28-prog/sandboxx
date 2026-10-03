#include "sandboxx/security_manager.hpp"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "[TEST] Running Security Manager Seccomp Policy Test...\n";

    sandboxx::SecurityPolicy policy;
    policy.enable_seccomp = true;
    policy.default_action = sandboxx::SeccompDefaultAction::KILL;
    policy.blocked_syscalls = {"ptrace", "reboot", "mount"};

    sandboxx::SecurityManager sec_mgr(policy);
    std::string dump = sec_mgr.export_bpf_filter_dump();
    assert(!dump.empty());

    std::cout << dump;
    std::cout << "[PASS] Security policy exported and verified.\n";
    return 0;
}
