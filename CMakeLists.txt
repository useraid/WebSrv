cmake_minimum_required(VERSION 3.10)

# Project
project(WebSrv)

# Version
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Link files
add_executable(server web.cpp WebSrv.cpp)

# Linkers
if(WIN32)
    target_link_libraries(server ws2_32)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(server Threads::Threads)
endif()
