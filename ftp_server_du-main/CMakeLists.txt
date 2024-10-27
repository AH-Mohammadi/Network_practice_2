cmake_minimum_required(VERSION 3.10)

# Set the project name
project(FileTransferServer)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add executable for the file transfer server, linking delete_file.cpp
add_executable(server server.cpp)

# Add executable for the main client
add_executable(client client.cpp)

# Add executable for the file list client
add_executable(file_list_client file_list_client.cpp)

# Add executable for the file search client
add_executable(file_search_client file_search_client.cpp)

# Add executable for the delete file client
add_executable(delete_file_client delete_file_client.cpp)
