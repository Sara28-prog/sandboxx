# SandBoxX Verification & Test Guide
**Muco Labs Engineering Series**

## Test Matrix
| Test Case | Target Program | Tested Subsystem | Expected Outcome |
|:---|:---|:---|:---|
| **TC-01** | `examples/hello.cpp` | PID Namespace | Process reports PID=1, PPID=0 inside sandbox |
| **TC-02** | `examples/hostname_test.cpp` | UTS Namespace | Container hostname changes; host hostname unchanged |
| **TC-03** | `examples/cpu_bomb.cpp` | Cgroup v2 `cpu.max` | Multi-threaded spin loop throttled to configured quota |
| **TC-04** | `examples/memory_bomb.cpp` | Cgroup v2 `memory.max` | OOM-killer fires at limit (Exit code 137 / SIGKILL) |
| **TC-05** | `examples/file_test.cpp` | `pivot_root` & Mount NS | Reading `/etc/shadow` or `../../` rejected (ENOENT/EACCES) |
| **TC-06** | Syscall Violation | Seccomp-BPF | Invoking `ptrace(2)` triggers immediate `SIGSYS (31)` |

## Execution Instructions
```bash
# Build sandboxx and example attack binaries
./scripts/build.sh

# Run TC-01 (Sanity Hello)
./build/sandboxx run --config configs/default.json --exec ./build/hello

# Run TC-03 (CPU Bomb)
./build/sandboxx run --config configs/restricted.json --exec ./build/cpu_bomb

# Run TC-04 (Memory Bomb)
./build/sandboxx run --config configs/restricted.json --exec ./build/memory_bomb

# Run TC-05 (Filesystem Escape Attempt)
./build/sandboxx run --config configs/restricted.json --exec ./build/file_test

# Automated unit & integration tests
ctest --test-dir build --output-on-failure
```
