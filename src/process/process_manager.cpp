#include "sandboxx/process_manager.hpp"
#include "sandboxx/logger.hpp"
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>
#include <csignal>
#include <cstring>

namespace sandboxx {

struct ChildContext {
    std::function<int()> entry_func;
    int p2c_pipe_read;
    int c2p_pipe_write;
};

static int child_trampoline(void* arg) {
    auto* ctx = static_cast<ChildContext*>(arg);

    // Wait for parent signal that cgroups and user maps are configured
    char buf[1];
    read(ctx->p2c_pipe_read, buf, 1);
    close(ctx->p2c_pipe_read);

    // Notify parent child has resumed
    write(ctx->c2p_pipe_write, "K", 1);
    close(ctx->c2p_pipe_write);

    // Execute sandbox child workload
    return ctx->entry_func();
}

ProcessManager::ProcessManager(const SandboxConfig& config)
    : config_(config) {
    // Allocate stack for clone()
    stack_mem_ = mmap(nullptr, stack_size_, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);

    // Create bidirectional synchronization pipes
    pipe(sync_pipe_p2c_);
    pipe(sync_pipe_c2p_);
}

ProcessManager::~ProcessManager() {
    if (stack_mem_ && stack_mem_ != MAP_FAILED) {
        munmap(stack_mem_, stack_size_);
    }
    if (sync_pipe_p2c_[0] >= 0) close(sync_pipe_p2c_[0]);
    if (sync_pipe_p2c_[1] >= 0) close(sync_pipe_p2c_[1]);
    if (sync_pipe_c2p_[0] >= 0) close(sync_pipe_c2p_[0]);
    if (sync_pipe_c2p_[1] >= 0) close(sync_pipe_c2p_[1]);
}

pid_t ProcessManager::spawn(std::function<int()> child_entry_func, int clone_flags) {
    if (stack_mem_ == MAP_FAILED) {
        LOG_ERROR("Failed to allocate stack memory for clone()");
        return -1;
    }

    void* stack_top = static_cast<char*>(stack_mem_) + stack_size_;

    ChildContext ctx;
    ctx.entry_func = child_entry_func;
    ctx.p2c_pipe_read = sync_pipe_p2c_[0];
    ctx.c2p_pipe_write = sync_pipe_c2p_[1];

    LOG_INFO("Invoking clone(2) with flags: 0x" + std::to_string(clone_flags));

    child_pid_ = clone(child_trampoline, stack_top, clone_flags | SIGCHLD, &ctx);

    if (child_pid_ < 0) {
        LOG_ERROR("clone() failed: " + std::string(strerror(errno)));
        return -1;
    }

    LOG_INFO("Child spawned successfully with host PID: " + std::to_string(child_pid_));
    return child_pid_;
}

bool ProcessManager::notify_child_ready() {
    if (sync_pipe_p2c_[1] >= 0) {
        write(sync_pipe_p2c_[1], "G", 1);
        close(sync_pipe_p2c_[1]);
        sync_pipe_p2c_[1] = -1;
        return true;
    }
    return false;
}

bool ProcessManager::wait_for_parent_ready() {
    if (sync_pipe_c2p_[0] >= 0) {
        char buf[1];
        read(sync_pipe_c2p_[0], buf, 1);
        close(sync_pipe_c2p_[0]);
        sync_pipe_c2p_[0] = -1;
        return true;
    }
    return false;
}

int ProcessManager::wait_for_exit(int* exit_signal, bool* oom_killed) {
    if (child_pid_ <= 0) return -1;

    int status = 0;
    waitpid(child_pid_, &status, 0);

    if (WIFEXITED(status)) {
        int code = WEXITSTATUS(status);
        LOG_INFO("Process exited normally with code " + std::to_string(code));
        return code;
    } else if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        if (exit_signal) *exit_signal = sig;
        if (sig == SIGKILL && oom_killed) {
            *oom_killed = true;
        }
        LOG_WARN("Process terminated by signal " + std::to_string(sig));
        return 128 + sig;
    }

    return -1;
}

bool ProcessManager::terminate(int signal) {
    if (child_pid_ > 0) {
        LOG_INFO("Sending signal " + std::to_string(signal) + " to PID " + std::to_string(child_pid_));
        return kill(child_pid_, signal) == 0;
    }
    return false;
}

bool ProcessManager::is_running() const {
    if (child_pid_ <= 0) return false;
    return kill(child_pid_, 0) == 0;
}

} // namespace sandboxx
