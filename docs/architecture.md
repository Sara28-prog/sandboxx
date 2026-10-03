# SandBoxX Architecture Specification
**Muco Labs — Linux Process Isolation & Resource Control System**

## 1. System Overview
SandBoxX is a lightweight, low-overhead container runtime and process isolation engine engineered in modern C++17. It enforces strict kernel-level isolation boundaries around untrusted executables without requiring a virtualization hypervisor or external daemon.

```
+-------------------------------------------------------------------+
|                        SandBoxX CLI / API                         |
+-------------------------------------------------------------------+
                                  |
                                  v
+-------------------------------------------------------------------+
|                     SandBoxX Runtime Engine                       |
|  +---------------------+  +-----------------+  +----------------+ |
|  |   ProcessManager    |  | CgroupManager   |  | SecurityMgr    | |
|  |  (clone, sync pipes)|  | (cgroups v2)    |  | (seccomp BPF)  | |
|  +---------------------+  +-----------------+  +----------------+ |
|  +---------------------+  +-----------------+  +----------------+ |
|  |  NamespaceManager   |  | FilesystemMgr   |  | Telemetry Mon  | |
|  | (PID,UTS,NS,IPC,NET)|  | (pivot_root/ro) |  | (proc/cgroups) | |
|  +---------------------+  +-----------------+  +----------------+ |
+-------------------------------------------------------------------+
                                  |
               [ Linux Kernel Subsystems & Primitives ]
                                  |
   +----------+  +----------+  +-----------+  +---------+  +--------+
   |   PID    |  |  Mount   |  |    UTS    |  | Cgroups |  |Seccomp |
   |Namespace |  |Namespace |  | Namespace |  |   v2    |  |  BPF   |
   +----------+  +----------+  +-----------+  +---------+  +--------+
                                  |
                                  v
                [ Isolated Sandboxed Process (PID 1) ]
```

## 2. Core Isolation Mechanisms

### 2.1 Linux Namespaces
SandBoxX isolates the execution context using `clone(2)` with the following flags:
- **PID Namespace (`CLONE_NEWPID`)**: The target process perceives itself as PID 1 (init). It cannot inspect or signal any processes running on the host OS.
- **Mount Namespace (`CLONE_NEWNS`)**: Mount propagation is marked private (`MS_REC | MS_PRIVATE`). Filesystem operations within the jail do not leak to the host.
- **UTS Namespace (`CLONE_NEWUTS`)**: Isolates hostname and domain name. Changes made via `sethostname` do not affect the host node.
- **IPC Namespace (`CLONE_NEWIPC`)**: Segregates System V IPC and POSIX message queues.
- **Network Namespace (`CLONE_NEWNET`)**: Disconnects from host network interfaces, isolating loopback.
- **User Namespace (`CLONE_NEWUSER`)**: Maps unprivileged host UID/GID to container root (UID 0).

### 2.2 Cgroups v2 Resource Accounting & Throttling
SandBoxX creates a dedicated control group under `/sys/fs/cgroup/sandboxx/<sandbox_id>`:
- **`cpu.max`**: Sets quota/period (e.g. `50000 100000` for 50% CPU allocation). Runaway infinite loops are throttled transparently.
- **`memory.max`**: Hard upper ceiling (e.g. 64MB). When breached, kernel out-of-memory killer triggers `SIGKILL (137)`.
- **`memory.high`**: Soft throttling threshold at 90% of max.
- **`memory.oom.group`**: Ensures the entire process tree inside the sandbox is terminated atomically on OOM.
- **`pids.max`**: Prevents fork-bomb exploits by limiting active threads and child processes.

### 2.3 Filesystem Jailing
- Uses `pivot_root(2)` with fallback to `chroot(2)`.
- Mounts minimal `/proc` and read-only `/sys`.
- Masks dangerous kernel files: `/proc/kcore`, `/proc/sysrq-trigger`, `/sys/firmware`.
- Root filesystem is mounted read-only (`MS_RDONLY`); `/tmp` is mounted on ephemeral memory `tmpfs`.

### 2.4 Security & System Call Filtering
- **`PR_SET_NO_NEW_PRIVS`**: Disallows `execve` from granting additional privileges via SUID binaries.
- **POSIX Capabilities**: Drops `CAP_SYS_ADMIN`, `CAP_NET_ADMIN`, `CAP_SYS_RAWIO`, etc.
- **Seccomp-BPF**: Enforces syscall whitelist/blacklist. Hazardous calls (`ptrace`, `reboot`, `kexec_load`, `mount`, `bpf`) are immediately terminated with `SIGSYS` (`SECCOMP_RET_KILL_PROCESS`).
