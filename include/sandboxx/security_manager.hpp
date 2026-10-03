#ifndef SANDBOXX_SECURITY_MANAGER_HPP
#define SANDBOXX_SECURITY_MANAGER_HPP

#include "sandbox_config.hpp"
#include <string>
#include <vector>
#include <seccomp.h>

namespace sandboxx {

class SecurityManager {
public:
    explicit SecurityManager(const SecurityPolicy& policy);
    ~SecurityManager();

    // Sets PR_SET_NO_NEW_PRIVS to prevent privilege escalation via setuid
    bool enforce_no_new_privs();

    // Drops all root and hazardous POSIX capabilities (e.g. CAP_SYS_ADMIN, CAP_NET_ADMIN)
    bool drop_capabilities();

    // Compiles and installs Seccomp-BPF filter using libseccomp
    bool apply_seccomp_filter();

    // Export BPF filter to BPF bytecode or readable disassembly for audit
    std::string export_bpf_filter_dump() const;

    const SecurityPolicy& get_policy() const { return policy_; }

private:
    SecurityPolicy policy_;
    scmp_filter_ctx seccomp_ctx_{nullptr};

    bool configure_whitelist_rules();
    bool configure_blacklist_rules();
};

} // namespace sandboxx

#endif // SANDBOXX_SECURITY_MANAGER_HPP
