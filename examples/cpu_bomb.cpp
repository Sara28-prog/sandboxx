#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>

// CPU Bomb: Spawns runaway threads calculating math to peg 100% of all cores
// SandBoxX cgroup cpu.max will clamp this to the configured quota (e.g. 20%)
int main() {
    std::cout << "[CPU Bomb] Launching aggressive multi-threaded CPU stress attack...\n"
              << "[CPU Bomb] Attempting to starve host CPU resources...\n";

    std::atomic<bool> keep_running{true};
    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4;

    std::cout << "[CPU Bomb] Spawning " << num_threads << " infinite loop worker threads.\n";

    std::vector<std::thread> workers;
    for (unsigned int i = 0; i < num_threads; ++i) {
        workers.emplace_back([&keep_running]() {
            volatile double x = 0.0;
            while (keep_running.load()) {
                x += 1.0;
                x = x * 1.000001;
            }
        });
    }

    // Run for 3 seconds under Cgroup throttling
    std::this_thread::sleep_for(std::chrono::seconds(3));
    keep_running.store(false);

    for (auto& t : workers) {
        if (t.joinable()) t.join();
    }

    std::cout << "[CPU Bomb] Attack phase completed. Cgroup cpu.max kept host CPU safe and responsive!\n";
    return 0;
}
