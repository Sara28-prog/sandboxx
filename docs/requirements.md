# SandBoxX Requirements Specification
**Muco Labs — Project 3: Linux Process Isolation & Resource Control System**

## Functional Requirements (FR)
- **FR-1 [Process Isolation]**: Untrusted binaries must execute in segregated namespaces (PID, Mount, UTS, IPC, Network, User).
- **FR-2 [CPU Control]**: Cgroups v2 must enforce CPU quota and period via `cpu.max`, preventing thread starvation on host cores.
- **FR-3 [Memory Control]**: Memory allocation must be strictly capped via `memory.max` and `memory.high`. Exceeding limits must trigger kernel OOM killer without affecting host memory.
- **FR-4 [Process Limiting]**: Maximum child processes/threads must be capped via `pids.max` to prevent fork-bombs.
- **FR-5 [Filesystem Jailing]**: Root filesystem must be isolated using `pivot_root` or `chroot`. Rootfs must be read-only; `/tmp` must be ephemeral tmpfs.
- **FR-6 [Syscall Filtering]**: Dangerous system calls (`ptrace`, `reboot`, `mount`, `kexec_load`, `bpf`) must be blocked using Seccomp-BPF filters.
- **FR-7 [CLI Management]**: Command-line interface providing `run`, `list`, `inspect`, `stats`, `logs`, `stop`, `kill`, and `test`.
- **FR-8 [Telemetry Monitoring]**: Background thread sampling CPU usage, memory RSS, and OOM kill events at configurable intervals.

## Non-Functional Requirements (NFR)
- **NFR-1 [Zero External Hardware]**: Must run entirely on standard Linux x86_64 / aarch64 kernels (>= Linux 5.4).
- **NFR-2 [Low Overhead]**: Startup latency < 15ms; telemetry overhead < 1% CPU.
- **NFR-3 [Crash Resilience]**: Parent must safely catch `SIGKILL`, `SIGSEGV`, `SIGSYS`, and `SIGALRM` timeouts without leaving dangling cgroups or mount leaks.
