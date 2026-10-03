#ifndef SANDBOXX_PROCESS_MANAGER_HPP
#define SANDBOXX_PROCESS_MANAGER_HPP

#include "sandbox_config.hpp"
#include <sys/types.h>
#include <functional>

namespace sandboxx {

class ProcessManager {
public:
    explicit ProcessManager(const SandboxConfig& config);
    ~ProcessManager();

    // Spawns the child process with clone() and configured namespace flags
    pid_t spawn(std::function<int()> child_entry_func, int clone_flags);

    // Waits for the child process to complete and captures exit status
    int wait_for_exit(int* exit_signal, bool* oom_killed);

    // Terminate with specific signal (SIGTERM, SIGKILL)
    bool terminate(int signal);

    pid_t get_child_pid() const { return child_pid_; }
    bool is_running() const;

    // Synchronize parent and child via unix pipes
    bool notify_child_ready();
    bool wait_for_parent_ready();

private:
    SandboxConfig config_;
    pid_t child_pid_{-1};
    int sync_pipe_p2c_[2]{-1, -1}; // parent to child sync pipe
    int sync_pipe_c2p_[2]{-1, -1}; // child to parent sync pipe
    void* stack_mem_{nullptr};
    size_t stack_size_{1024 * 1024}; // 1MB stack for clone()
};

} // namespace sandboxx

#endif // SANDBOXX_PROCESS_MANAGER_HPP
