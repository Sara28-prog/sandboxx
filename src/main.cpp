#include "sandboxx/cli.hpp"
#include "sandboxx/logger.hpp"
#include "sandboxx/utils.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    // Initialize logger for CLI
    sandboxx::Logger::instance().init("", sandboxx::LogLevel::INFO);

    if (argc < 2) {
        sandboxx::cli::print_usage();
        return 1;
    }

    try {
        sandboxx::cli::CLI cli_handler;
        return cli_handler.execute(argc, argv);
    } catch (const std::exception& ex) {
        LOG_FATAL(std::string("Fatal sandboxx error: ") + ex.what());
        std::cerr << "SandBoxX Fatal: " << ex.what() << std::endl;
        return 1;
    }
}
