#ifndef SANDBOXX_NAMESPACE_MANAGER_HPP
#define SANDBOXX_NAMESPACE_MANAGER_HPP

#include "sandbox_config.hpp"
#include <sched.h>
#include <sys/types.h>
#include <string>

namespace sandboxx {

class NamespaceManager {
public:
    explicit NamespaceManager(const NamespaceConfig& config);
    ~NamespaceManager();

    // Returns the combined bitwise flags for clone(2)
    int get_clone_flags() const;

    // Apply namespaces from inside the child process
    bool setup_child_namespaces();

    // Setup host-side namespace bindings, UID/GID mappings, veth interfaces
    bool setup_parent_namespaces(pid_t child_pid);

    // Isolate hostname in UTS namespace
    bool configure_uts();

    // Setup IPC message queue isolation
    bool configure_ipc();

    // Setup network loopback or dummy interface
    bool configure_network();

    // UID/GID mapping for user namespace
    bool configure_user_mappings(pid_t child_pid);

private:
    NamespaceConfig config_;
};

} // namespace sandboxx

#endif // SANDBOXX_NAMESPACE_MANAGER_HPP
