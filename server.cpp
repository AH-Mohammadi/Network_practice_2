#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fstream>
#include <vector>
#include <filesystem>

#define PORT 8080

void handleClient(int client_socket) {
    char buffer[1024] = {0};
    while (true) {
        int bytes_read = read(client_socket, buffer, sizeof(buffer));
        if (bytes_read <= 0) {
            break; // Exit if no data is read
        }

        buffer[bytes_read] = '\0'; // Null-terminate the buffer
        std::string command(buffer);

        if (command == "UPLOAD") {
            // Handle file upload
            // Receive the filename
            read(client_socket, buffer, sizeof(buffer));
            std::string filename(buffer);
            std::ofstream outfile(filename, std::ios::binary);

            // Read the file content
            while ((bytes_read = read(client_socket, buffer, sizeof(buffer))) > 0) {
                outfile.write(buffer, bytes_read);
            }
            std::cout << "Uploaded file: " << filename << "\n";
            outfile.close();
        } else if (command == "DOWNLOAD") {
            // Handle file download
            read(client_socket, buffer, sizeof(buffer));
            std::string filename(buffer);
            std::ifstream infile(filename, std::ios::binary);
            if (infile) {
                while (infile.read(buffer, sizeof(buffer))) {
                    send(client_socket, buffer, infile.gcount(), 0);
                }
                // Send any remaining bytes
                if (infile.gcount() > 0) {
                    send(client_socket, buffer, infile.gcount(), 0);
                }
                std::cout << "Sent file: " << filename << "\n";
            } else {
                std::cerr << "File not found: " << filename << "\n";
            }
            infile.close();
        } else if (command == "DELETE_FILE") {
            // Handle file deletion
            read(client_socket, buffer, sizeof(buffer));
            std::string filename(buffer);
            if (std::remove(filename.c_str()) == 0) {
                std::cout << "Deleted file: " << filename << "\n";
                send(client_socket, "File deleted successfully", 25, 0);
            } else {
                send(client_socket, "File deletion failed", 20, 0);
            }
        } else if (command == "GET_FILE_LIST") {
            // List files in the current directory
            std::vector<std::string> files;
            for (const auto& entry : std::filesystem::directory_iterator(".")) {
                files.push_back(entry.path().filename().string());
            }
            std::string fileList = "Files:\n" + std::accumulate(files.begin(), files.end(), std::string(),
                [](std::string a, std::string b) { return std::move(a) + b + "\n"; });
            send(client_socket, fileList.c_str(), fileList.length(), 0);
        } else if (command == "SEARCH_FILE") {
            // Handle file search
            read(client_socket, buffer, sizeof(buffer));
            std::string filename(buffer);
            if (std::filesystem::exists(filename)) {
                send(client_socket, "File found", 10, 0);
            } else {
                send(client_socket, "File not found", 14, 0);
            }
        }
    }
    close(client_socket);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the address
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << PORT << "\n";

    while (true) {
        // Accept a new connection
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        std::cout << "New connection established\n";
        handleClient(new_socket);
    }

    return 0;
}
