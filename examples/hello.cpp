#include <iostream>
#include <unistd.h>
#include <sys/types.h>

int main() {
    char hostname[256];
    gethostname(hostname, sizeof(hostname));

    std::cout << "--- SandBoxX Hello World Test ---\n"
              << "Isolated Process Information:\n"
              << "  Inside Container PID: " << getpid() << " (Should be 1 in private PID namespace)\n"
              << "  Parent PID:           " << getppid() << "\n"
              << "  Container UID:        " << getuid() << "\n"
              << "  Container Hostname:   " << hostname << "\n"
              << "Execution completed normally inside sandbox.\n";
    return 0;
}
