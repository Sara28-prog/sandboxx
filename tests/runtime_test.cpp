#include "sandboxx/runtime.hpp"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "[TEST] Running SandBoxX Runtime Lifecycle Test...\n";

    sandboxx::Runtime& rt = sandboxx::Runtime::get_instance();
    sandboxx::SandboxConfig cfg = sandboxx::SandboxConfig::create_restricted();
    cfg.binary_path = "/bin/true";

    std::string id = rt.create_sandbox(cfg);
    assert(!id.empty());

    auto sb = rt.get_sandbox(id);
    assert(sb != nullptr);
    assert(sb->get_state() == sandboxx::SandboxState::CREATED);

    std::cout << "[PASS] Sandbox runtime created instance: " << id << " successfully.\n";
    return 0;
}
