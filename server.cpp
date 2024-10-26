#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <filesystem>
#include <vector>

#define PORT 8080

void handleUpload(int new_socket) {
    char buffer[1024] = {0};

    // Receive filename
    read(new_socket, buffer, sizeof(buffer));
    std::string filename(buffer);

    // Open the file to write
    std::ofstream outfile(filename, std::ios::binary);
    if (!outfile) {
        std::cerr << "Failed to create file: " << filename << "\n";
        return;
    }

    // Receive file data
    while (true) {
        int bytes_read = read(new_socket, buffer, sizeof(buffer));
        if (bytes_read <= 0) break;  // Exit on error or EOF
        outfile.write(buffer, bytes_read);
    }

    std::cout << "File uploaded successfully: " << filename << "\n";
    outfile.close();
}

void handleDownload(int new_socket) {
    char buffer[1024] = {0};

    // Receive filename
    read(new_socket, buffer, sizeof(buffer));
    std::string filename(buffer);

    // Check if file exists
    if (!std::filesystem::exists(filename)) {
        std::cerr << "File not found: " << filename << "\n";
        return;
    }

    // Send file data
    std::ifstream infile(filename, std::ios::binary);
    while (infile.read(buffer, sizeof(buffer))) {
        send(new_socket, buffer, infile.gcount(), 0);
    }
    // Send any remaining bytes
    if (infile.gcount() > 0) {
        send(new_socket, buffer, infile.gcount(), 0);
    }

    std::cout << "File downloaded successfully: " << filename << "\n";
    infile.close();
}

void handleDelete(int new_socket) {
    char buffer[1024] = {0};

    // Receive filename
    read(new_socket, buffer, sizeof(buffer));
    std::string filename(buffer);

    // Delete the file
    if (std::filesystem::remove(filename)) {
        std::cout << "File deleted successfully: " << filename << "\n";
        const char* response = "File deleted successfully.";
        send(new_socket, response, strlen(response), 0);
    } else {
        std::cerr << "Failed to delete file: " << filename << "\n";
        const char* response = "Failed to delete file.";
        send(new_socket, response, strlen(response), 0);
    }
}

void handleListFiles(int new_socket) {
    std::string filesList;
    for (const auto& entry : std::filesystem::directory_iterator(".")) {
        filesList += entry.path().filename().string() + "\n";
    }

    send(new_socket, filesList.c_str(), filesList.size(), 0);
    std::cout << "File list sent to client.\n";
}

void handleSearchFile(int new_socket) {
    char buffer[1024] = {0};

    // Receive filename
    read(new_socket, buffer, sizeof(buffer));
    std::string filename(buffer);

    if (std::filesystem::exists(filename)) {
        std::cout << "File found: " << filename << "\n";
        const char* response = "File exists on the server.";
        send(new_socket, response, strlen(response), 0);
    } else {
        std::cerr << "File not found: " << filename << "\n";
        const char* response = "File does not exist on the server.";
        send(new_socket, response, strlen(response), 0);
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation error\n";
        return -1;
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::cerr << "Set socket options error\n";
        return -1;
    }

    // Configure server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Binding failed\n";
        return -1;
    }

    // Start listening
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listening failed\n";
        return -1;
    }

    std::cout << "Server is listening on port " << PORT << "\n";

    while (true) {
        // Accept a new connection
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Accepting connection failed\n";
            continue;
        }

        // Handle client commands
        char buffer[1024] = {0};
        read(new_socket, buffer, sizeof(buffer));
        std::string command(buffer);

        if (command == "UPLOAD") {
            handleUpload(new_socket);
        } else if (command == "DOWNLOAD") {
            handleDownload(new_socket);
        } else if (command == "DELETE_FILE") {
            handleDelete(new_socket);
        } else if (command == "GET_FILE_LIST") {
            handleListFiles(new_socket);
        } else if (command == "SEARCH_FILE") {
            handleSearchFile(new_socket);
        } else {
            std::cerr << "Unknown command received\n";
        }

        // Close the client socket
        close(new_socket);
    }

    // Close the server socket
    close(server_fd);
    return 0;
}
