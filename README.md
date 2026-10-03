# SandBoxX
### Linux Process Isolation & Resource Control System
**Developed by Muco Labs**

SandBoxX is a high-performance, Linux-native process sandbox developed in C++17. It isolates untrusted programs from the host operating system using modern Linux kernel facilities:
- **Kernel Namespaces**: Segregates Process IDs (`CLONE_NEWPID`), Mount points (`CLONE_NEWNS`), Hostname (`CLONE_NEWUTS`), IPC queues (`CLONE_NEWIPC`), Network stacks (`CLONE_NEWNET`), and Users (`CLONE_NEWUSER`).
- **Control Groups v2 (cgroups)**: Enforces precise CPU quotas (`cpu.max`), hard physical RAM limits (`memory.max`), soft throttle thresholds (`memory.high`), and process bounds (`pids.max`).
- **Filesystem Jailing**: Establishes hermetic isolation using `pivot_root(2)` with read-only root mounts and ephemeral `tmpfs` scratchpads.
- **Seccomp-BPF Syscall Filtering**: Restricts kernel syscall dispatch, dropping risky calls like `ptrace`, `reboot`, `mount`, and `kexec_load`.
- **Command-Line Interface**: Ergonomic CLI for starting, stopping, inspecting, monitoring, and debugging isolated processes.

---

## Directory Structure
```
sandboxx/
├── CMakeLists.txt
├── README.md
├── LICENSE
├── include/sandboxx/
│   ├── sandbox_config.hpp
│   ├── runtime.hpp
│   ├── process_manager.hpp
│   ├── namespace_manager.hpp
│   ├── cgroup_manager.hpp
│   ├── filesystem_manager.hpp
│   ├── security_manager.hpp
│   ├── monitor.hpp
│   ├── logger.hpp
│   ├── utils.hpp
│   └── cli.hpp
├── src/
│   ├── main.cpp
│   ├── cli/
│   ├── runtime/
│   ├── process/
│   ├── namespace/
│   ├── cgroup/
│   ├── filesystem/
│   ├── security/
│   ├── monitor/
│   └── common/
├── configs/
│   ├── default.json
│   ├── restricted.json
│   └── development.json
├── rootfs/
├── examples/
│   ├── hello.cpp
│   ├── cpu_bomb.cpp
│   ├── memory_bomb.cpp
│   ├── file_test.cpp
│   └── hostname_test.cpp
├── tests/
├── scripts/
└── docs/
```

---

## Quick Start
```bash
# 1. Compile SandBoxX and test targets
./scripts/build.sh

# 2. Run hello world inside sandbox
sudo ./build/sandboxx run --config configs/default.json --exec ./build/hello

# 3. Test runaway memory bomb (contained safely by Cgroup OOM killer)
sudo ./build/sandboxx run --config configs/restricted.json --exec ./build/memory_bomb

# 4. View active/past sandboxes
./build/sandboxx list
```
