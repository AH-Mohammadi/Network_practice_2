# CMake minimum version
cmake_minimum_required(VERSION 3.10)

# Project name
project(FileTransferSystem)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add executable for the server
add_executable(server server.cpp)

# Add executable for the client
add_executable(client client.cpp)

# Link libraries (if required for networking on your system)
# Uncomment if necessary
# target_link_libraries(server pthread)
# target_link_libraries(client pthread)
