# SandBoxX Design Document
**Muco Labs Engineering Series — Process Sandboxing Specification**

## 1. Process Lifecycle State Machine
```
[CREATED] ---> [INITIALIZING] ---> [RUNNING] ---> [EXITED]
                     |                  |
                     v                  +-----> [KILLED_OOM]
                 [ERROR]                |
                                        +-----> [KILLED_SECCOMP]
                                        |
                                        +-----> [STOPPED]
```

### State Definitions
1. **CREATED**: Sandbox configuration loaded, resource structures instantiated.
2. **INITIALIZING**: Cgroups created, mount points mapped, sync pipes initialized.
3. **RUNNING**: `clone()` child spawned, parent attached child to cgroup, telemetry active.
4. **EXITED**: Process terminated with standard exit status (0 or user error).
5. **KILLED_OOM**: Kernel memory controller triggered Out-Of-Memory termination (Exit 137).
6. **KILLED_SECCOMP**: Process attempted disallowed syscall, caught by BPF filter (SIGSYS 31).
7. **STOPPED**: Terminated by user signal (SIGTERM or SIGKILL).

## 2. Parent-Child Synchronization Protocol
To prevent race conditions where child starts executing untrusted code before parent attaches it to the Cgroup hierarchy or configures User Namespace UID maps:
1. Parent creates two unidirectional UNIX pipes: `p2c` (Parent-to-Child) and `c2p` (Child-to-Parent).
2. `clone()` executes child trampoline function `child_trampoline()`.
3. Child blocks on `read(p2c[0], &buf, 1)`.
4. Parent executes `cgroup_mgr_->attach_process(child_pid)` and writes `/proc/child_pid/uid_map`.
5. Parent unblocks child by writing to `p2c[1]`.
6. Child receives token, applies `pivot_root`, loads Seccomp-BPF filter, and executes `execv()`.
