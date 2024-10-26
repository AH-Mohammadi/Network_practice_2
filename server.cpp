#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <dirent.h>
#include <sys/stat.h>

#define PORT 8080

void handleUpload(int new_socket) {
    char filename[1024] = {0};
    read(new_socket, filename, sizeof(filename));

    std::ofstream outfile(filename, std::ios::binary);
    if (!outfile) {
        std::cerr << "Failed to create file: " << filename << "\n";
        return;
    }

    char file_buffer[1024];
    int bytes = 0;
    while ((bytes = read(new_socket, file_buffer, sizeof(file_buffer))) > 0) {
        outfile.write(file_buffer, bytes);
    }

    std::cout << "File uploaded successfully as " << filename << "\n";
    outfile.close();
}

void handleDownload(int new_socket) {
    char filename[1024] = {0};
    read(new_socket, filename, sizeof(filename));

    std::ifstream infile(filename, std::ios::binary);
    if (!infile) {
        std::cerr << "File not found: " << filename << "\n";
        return;
    }

    char file_buffer[1024];
    int bytes = 0;
    while ((bytes = infile.readsome(file_buffer, sizeof(file_buffer))) > 0) {
        send(new_socket, file_buffer, bytes, 0);
    }

    std::cout << "File sent successfully: " << filename << "\n";
    infile.close();
}

void handleFileList(int new_socket) {
    const char* directoryPath = ".";
    DIR* dir;
    struct dirent* ent;

    if ((dir = opendir(directoryPath)) != nullptr) {
        std::string fileList;

        while ((ent = readdir(dir)) != nullptr) {
            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                fileList += ent->d_name;
                fileList += "\n";
            }
        }

        closedir(dir);
        send(new_socket, fileList.c_str(), fileList.size(), 0);
        std::cout << "Sent file list to client:\n" << fileList;
    } else {
        std::cerr << "Could not open directory: " << directoryPath << "\n";
    }
}

void handleSearchFile(int new_socket) {
    char filename[1024] = {0};
    read(new_socket, filename, sizeof(filename));

    std::ifstream infile(filename);
    const char* response = infile ? "File found!" : "File not found!";
    send(new_socket, response, strlen(response), 0);
    infile.close();
}

void handleDeleteFile(int new_socket) {
    char filename[1024] = {0};
    read(new_socket, filename, sizeof(filename));

    if (remove(filename) == 0) {
        const char* response = "File deleted successfully.";
        send(new_socket, response, strlen(response), 0);
        std::cout << "Deleted file: " << filename << "\n";
    } else {
        const char* response = "Failed to delete file.";
        send(new_socket, response, strlen(response), 0);
        std::cerr << "Error deleting file: " << filename << "\n";
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
        std::cerr << "setsockopt error\n";
        return -1;
    }

    // Configure server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed\n";
        return -1;
    }

    // Start listening
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed\n";
        return -1;
    }

    while (true) {
        std::cout << "Waiting for a connection...\n";
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Accept failed\n";
            return -1;
        }

        char command[1024] = {0};
        read(new_socket, command, sizeof(command));

        if (strcmp(command, "UPLOAD") == 0) {
            handleUpload(new_socket);
        } else if (strcmp(command, "DOWNLOAD") == 0) {
            handleDownload(new_socket);
        } else if (strcmp(command, "GET_FILE_LIST") == 0) {
            handleFileList(new_socket);
        } else if (strcmp(command, "SEARCH_FILE") == 0) {
            handleSearchFile(new_socket);
        } else if (strcmp(command, "DELETE_FILE") == 0) {
            handleDeleteFile(new_socket);
        } else {
            std::cerr << "Unknown command received: " << command << "\n";
        }

        close(new_socket);
    }

    close(server_fd);
    return 0;
}
