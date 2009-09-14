cmake_minimum_required (VERSION 2.6)
project (cpuid)

include($ENV{HOME}/Code/CMake/CMakeUserConfig.cmake)

add_executable(cpuid src/cpuid.cpp)
