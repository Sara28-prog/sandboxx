#include "sandboxx/cli.hpp"
#include <iostream>
#include <cstring>

namespace sandboxx {
namespace cli {

ParsedCommand CommandParser::parse(int argc, char* argv[]) {
    ParsedCommand cmd;
    if (argc < 2) return cmd;

    cmd.command = argv[1];

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--config" || arg == "-c") {
            if (i + 1 < argc) cmd.config_file = argv[++i];
        } else if (arg == "--exec" || arg == "-e") {
            if (i + 1 < argc) cmd.executable = argv[++i];
        } else if (arg == "--cpu") {
            if (i + 1 < argc) cmd.cpu_quota = std::stoul(argv[++i]);
        } else if (arg == "--memory" || arg == "-m") {
            if (i + 1 < argc) cmd.memory_limit_mb = std::stoull(argv[++i]);
        } else if (arg == "--pids") {
            if (i + 1 < argc) cmd.max_pids = std::stoul(argv[++i]);
        } else if (arg == "--timeout" || arg == "-t") {
            if (i + 1 < argc) cmd.timeout_sec = std::stoul(argv[++i]);
        } else if (arg == "--rootfs") {
            if (i + 1 < argc) cmd.rootfs_dir = argv[++i];
        } else if (arg == "--verbose" || arg == "-v") {
            cmd.verbose = true;
        } else if (arg == "--follow" || arg == "-f") {
            cmd.follow = true;
        } else if (arg == "--signal" || arg == "-s") {
            if (i + 1 < argc) cmd.signal = std::stoi(argv[++i]);
        } else if (cmd.sandbox_id.empty() && arg[0] != '-') {
            cmd.sandbox_id = arg;
        } else {
            cmd.exec_args.push_back(arg);
        }
    }

    return cmd;
}

void print_usage() {
    std::cout << "SandBoxX — Linux Process Isolation & Resource Control System\n"
              << "Version 1.0.0 (Muco Labs)\n\n"
              << "USAGE:\n"
              << "  sandboxx <command> [options]\n\n"
              << "COMMANDS:\n"
              << "  run       Execute a binary inside an isolated sandbox\n"
              << "  list      List all active and recent sandboxes\n"
              << "  inspect   Display full configuration and kernel namespace metadata\n"
              << "  stats     Stream live CPU, memory, and I/O metrics\n"
              << "  logs      Show stdout/stderr and audit trails for a sandbox\n"
              << "  stop      Gracefully terminate a running sandbox (SIGTERM)\n"
              << "  kill      Forcefully terminate a running sandbox (SIGKILL)\n"
              << "  test      Run test suite demonstrating isolation and bomb containment\n"
              << "  version   Display version and Linux kernel capability support\n\n"
              << "OPTIONS FOR 'run':\n"
              << "  --config, -c <file>     Load sandbox configuration JSON\n"
              << "  --exec, -e <path>       Executable to run in sandbox\n"
              << "  --cpu <percent>         Set CPU quota percentage (default: 50%)\n"
              << "  --memory, -m <mb>       Set memory limit in megabytes (default: 128MB)\n"
              << "  --pids <count>          Maximum allowed process/thread count\n"
              << "  --timeout, -t <sec>     Execution timeout in seconds\n"
              << "  --rootfs <path>         Path to rootfs directory\n"
              << "  --verbose, -v           Enable verbose debug logs\n";
}

void print_version() {
    std::cout << "SandBoxX v1.0.0 (C++17, Linux Namespaces, Cgroups v2, Seccomp-BPF)\n"
              << "Engineered by Muco Labs.\n";
}

} // namespace cli
} // namespace sandboxx
