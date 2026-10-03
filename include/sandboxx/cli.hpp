#ifndef SANDBOXX_CLI_HPP
#define SANDBOXX_CLI_HPP

#include "sandbox_config.hpp"
#include <string>
#include <vector>

namespace sandboxx {
namespace cli {

struct ParsedCommand {
    std::string command;
    std::string config_file;
    std::string executable;
    std::vector<std::string> exec_args;
    std::string sandbox_id;
    std::string rootfs_dir;
    uint32_t cpu_quota{50};
    uint64_t memory_limit_mb{128};
    uint32_t max_pids{32};
    uint32_t timeout_sec{30};
    bool verbose{false};
    bool follow{false};
    int signal{15};
};

class CommandParser {
public:
    static ParsedCommand parse(int argc, char* argv[]);
};

void print_usage();
void print_version();

class CLI {
public:
    CLI() = default;
    int execute(int argc, char* argv[]);

private:
    int handle_run(const ParsedCommand& cmd);
    int handle_list();
    int handle_inspect(const std::string& id);
    int handle_stop(const std::string& id, int sig);
    int handle_kill(const std::string& id);
    int handle_stats(const std::string& id, bool follow);
    int handle_logs(const std::string& id);
    int handle_test();
};

} // namespace cli
} // namespace sandboxx

#endif // SANDBOXX_CLI_HPP
