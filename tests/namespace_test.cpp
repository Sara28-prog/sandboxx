#include "sandboxx/namespace_manager.hpp"
#include <iostream>
#include <cassert>
#include <sched.h>

int main() {
    std::cout << "[TEST] Running Namespace Manager Test...\n";

    sandboxx::NamespaceConfig config;
    config.enable_pid = true;
    config.enable_mount = true;
    config.enable_uts = true;
    config.enable_ipc = true;
    config.enable_net = true;
    config.hostname = "test-node-01";

    sandboxx::NamespaceManager mgr(config);
    int flags = mgr.get_clone_flags();

    assert((flags & CLONE_NEWPID) != 0);
    assert((flags & CLONE_NEWNS) != 0);
    assert((flags & CLONE_NEWUTS) != 0);
    assert((flags & CLONE_NEWIPC) != 0);
    assert((flags & CLONE_NEWNET) != 0);

    std::cout << "[PASS] Namespace clone flags verified (0x" << std::hex << flags << std::dec << ")\n";
    return 0;
}
