cmake_minimum_required(VERSION 3.16)
project(SandBoxX VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -pthread")

# Dependencies: libseccomp for syscall filtering, pthread
find_package(PkgConfig REQUIRED)
pkg_check_modules(SECCOMP REQUIRED libseccomp)

include_directories(
    include
    ${SECCOMP_INCLUDE_DIRS}
)

# Core library sources
set(SANDBOXX_SOURCES
    src/common/logger.cpp
    src/common/utils.cpp
    src/cgroup/cgroup_manager.cpp
    src/namespace/namespace_manager.cpp
    src/filesystem/filesystem_manager.cpp
    src/security/security_manager.cpp
    src/process/process_manager.cpp
    src/monitor/monitor.cpp
    src/runtime/runtime.cpp
    src/cli/command_parser.cpp
    src/cli/cli.cpp
)

add_library(sandboxx_core STATIC ${SANDBOXX_SOURCES})
target_link_libraries(sandboxx_core ${SECCOMP_LIBRARIES} pthread)

# Main CLI executable
add_executable(sandboxx src/main.cpp)
target_link_libraries(sandboxx sandboxx_core ${SECCOMP_LIBRARIES} pthread)

# Example target binaries
add_executable(hello examples/hello.cpp)
add_executable(cpu_bomb examples/cpu_bomb.cpp)
add_executable(memory_bomb examples/memory_bomb.cpp)
add_executable(file_test examples/file_test.cpp)
add_executable(hostname_test examples/hostname_test.cpp)

# Tests
enable_testing()
add_executable(namespace_test tests/namespace_test.cpp)
target_link_libraries(namespace_test sandboxx_core ${SECCOMP_LIBRARIES} pthread)
add_test(NAME NamespaceTest COMMAND namespace_test)

add_executable(cgroup_test tests/cgroup_test.cpp)
target_link_libraries(cgroup_test sandboxx_core ${SECCOMP_LIBRARIES} pthread)
add_test(NAME CgroupTest COMMAND cgroup_test)

add_executable(filesystem_test tests/filesystem_test.cpp)
target_link_libraries(filesystem_test sandboxx_core ${SECCOMP_LIBRARIES} pthread)
add_test(NAME FilesystemTest COMMAND filesystem_test)

add_executable(security_test tests/security_test.cpp)
target_link_libraries(security_test sandboxx_core ${SECCOMP_LIBRARIES} pthread)
add_test(NAME SecurityTest COMMAND security_test)

add_executable(runtime_test tests/runtime_test.cpp)
target_link_libraries(runtime_test sandboxx_core ${SECCOMP_LIBRARIES} pthread)
add_test(NAME RuntimeTest COMMAND runtime_test)
