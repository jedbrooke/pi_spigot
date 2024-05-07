cmake_minimum_required(VERSION 3.7)

set(CMAKE_OSX_ARCHITECTURES "x86_64")
set(CMAKE_OSX_ARCHITECTURES "arm64")

project(pi_spigot)

set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_FLAGS "-g")
set(CMAKE_CXX_FLAGS "-O3")

add_subdirectory(fractionalBignum)
include_directories(fractionalBignum)

add_executable(pi_spigot modpow.cpp modpow.h pi_spigot.cpp Worker.cpp Worker.hpp WorkManager.cpp WorkManager.hpp)
target_link_libraries(pi_spigot fractionalBignum pthread m)
