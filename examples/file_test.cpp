#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>

// File Test: Attempts to escape filesystem jail and read /etc/shadow or modify host root
int main() {
    std::cout << "--- SandBoxX Filesystem Jail Security Test ---\n";

    // 1. Attempt reading /etc/shadow
    std::cout << "[Test 1] Attempting to read /etc/shadow...\n";
    std::ifstream shadow("/etc/shadow");
    if (shadow.is_open()) {
        std::cout << "  [CRITICAL LEAK] Succeeded in reading /etc/shadow!\n";
    } else {
        std::cout << "  [SECURE] Access to /etc/shadow blocked or file does not exist in jailed rootfs.\n";
    }

    // 2. Attempt path traversal escape: ../../../etc/passwd
    std::cout << "[Test 2] Attempting path traversal escape: ../../../etc/passwd...\n";
    std::ifstream escape("../../../etc/passwd");
    if (escape.is_open()) {
        std::cout << "  [RESULT] Read file through path traversal.\n";
    } else {
        std::cout << "  [SECURE] Traversal clamped by pivot_root / chroot jail.\n";
    }

    // 3. Attempt writing to root /
    std::cout << "[Test 3] Attempting to create unauthorized file at /pwned.txt...\n";
    std::ofstream malicious("/pwned.txt");
    if (malicious.is_open()) {
        malicious << "jailbroken";
        std::cout << "  [WARNING] Root is writable.\n";
    } else {
        std::cout << "  [SECURE] Root filesystem is mounted read-only (EPERM / EROFS).\n";
    }

    std::cout << "Filesystem jail security test completed.\n";
    return 0;
}
