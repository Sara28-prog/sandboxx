#include <iostream>
#include <unistd.h>
#include <cstring>

// Hostname Test: Attempts to overwrite system hostname
// In UTS namespace, this only affects the sandbox; host hostname remains untouched
int main() {
    char old_name[256];
    gethostname(old_name, sizeof(old_name));
    std::cout << "[UTS Test] Current container hostname: " << old_name << "\n";

    std::cout << "[UTS Test] Attempting sethostname('compromised-node')...\n";
    const char* attack_name = "compromised-node";
    if (sethostname(attack_name, strlen(attack_name)) == 0) {
        char new_name[256];
        gethostname(new_name, sizeof(new_name));
        std::cout << "  [SUCCESS] Container hostname updated to: " << new_name << "\n"
                  << "  [ISOLATION] Host UTS namespace remains completely unchanged.\n";
    } else {
        std::cout << "  [FAILED] sethostname failed: " << strerror(errno) << "\n";
    }

    return 0;
}
