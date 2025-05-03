cmake_minimum_required(VERSION 3.14)
project(tcp CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
add_compile_options(-Wall -Wextra -Wpedantic -DALLOW_EXPERIMENTAL_API)

# Recursively include all directories under include/
file(GLOB_RECURSE HEADERS "include/*.h" "include/*.hpp")
foreach(header ${HEADERS})
    get_filename_component(dir ${header} DIRECTORY)
    include_directories(${dir})
endforeach()

# Recursively include all directories under src/ (for includes like #include "net/tcp.h")
file(GLOB_RECURSE SRC_HEADERS "src/*.h" "src/*.hpp")
foreach(header ${SRC_HEADERS})
    get_filename_component(dir ${header} DIRECTORY)
    include_directories(${dir})
endforeach()

# Gather all source files recursively from src/
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Find DPDK via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(DPDK REQUIRED libdpdk)

include_directories(${DPDK_INCLUDE_DIRS})
link_directories(${DPDK_LIBRARY_DIRS})
add_definitions(${DPDK_CFLAGS_OTHER})

# Define the executable target
add_executable(tcp ${SOURCES})

# Link libraries
target_link_libraries(tcp ${DPDK_LIBRARIES} pthread)
