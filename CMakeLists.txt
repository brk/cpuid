cmake_minimum_required (VERSION 2.6)

project (cpuid)

add_library(cpuid STATIC src/cpuid.cpp)

add_executable(testcpuid src/cpuid_main.cpp src/jsoncpp-fused.cpp)

target_link_libraries(testcpuid cpuid)
