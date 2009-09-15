cmake_minimum_required (VERSION 2.6)
project (cpuid)

include($ENV{HOME}/Code/CMake/CMakeUserConfig.cmake OPTIONAL)

include(${CMAKE_INSTALL_PREFIX}/lib/jsoncpp/jsoncpp-targets.cmake)

add_executable(cpuid src/cpuid.cpp)

target_link_libraries(cpuid json)
