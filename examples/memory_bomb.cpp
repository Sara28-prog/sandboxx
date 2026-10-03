#include <iostream>
#include <vector>
#include <cstring>
#include <chrono>
#include <thread>

// Memory Bomb: Continuously allocates memory in 10MB chunks and touches each page
// When it exceeds cgroup memory.max (e.g. 64MB), Linux Kernel OOM-Killer fires
// sending SIGKILL (exit code 137). SandBoxX captures this and protects the host.
int main() {
    std::cout << "[Memory Bomb] Launching exponential memory allocation attack...\n"
              << "[Memory Bomb] Target: Exhaust host RAM and crash system...\n";

    std::vector<char*> allocations;
    const size_t chunk_size = 10 * 1024 * 1024; // 10 MB per step
    size_t total_allocated = 0;

    for (int step = 1; step <= 50; ++step) {
        char* ptr = static_cast<char*>(malloc(chunk_size));
        if (!ptr) {
            std::cout << "[Memory Bomb] malloc() failed after " << (total_allocated / (1024 * 1024)) << " MB.\n";
            break;
        }

        // Must touch pages with memset to force physical page faulting / RSS increase
        memset(ptr, 0xAA, chunk_size);
        allocations.push_back(ptr);
        total_allocated += chunk_size;

        std::cout << "[Memory Bomb] Allocated & dirtied: " << (total_allocated / (1024 * 1024)) 
                  << " MB physical RAM (Iteration " << step << ")\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "[Memory Bomb] Done.\n";
    return 0;
}
