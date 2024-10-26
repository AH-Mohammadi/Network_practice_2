#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <filesystem>
#include <fstream>

#define PORT 8080

void handleClient(int client_socket) {
    char buffer[1024] = {0};
    int bytes_read;

    while ((bytes_read = read(client_socket, buffer, sizeof(buffer))) > 0) {
        buffer[bytes_read] = '\0'; // Null-terminate the received string
        std::string command(buffer);

        if (command == "UPLOAD") {
            // Get the filename
            read(client_socket, buffer, sizeof(buffer));
            std::string filename(buffer);

            // Create or open the file for writing
            std::ofstream outfile("server/" + filename, std::ios::binary);
            if (!outfile.is_open()) {
                std::cerr << "Failed to open file for upload\n";
                continue;
            }

            // Receive the file content
            while ((bytes_read = read(client_socket, buffer, sizeof(buffer))) > 0) {
                outfile.write(buffer, bytes_read);
            }
            std::cout << "File uploaded successfully: " << filename << "\n";
            outfile.close();
        } else if (command == "DOWNLOAD") {
            // Get the filename
            read(client_socket, buffer, sizeof(buffer));
            std::string filename(buffer);
            std::ifstream infile("server/" + filename, std::ios::binary);
            
            if (infile.is_open()) {
                // Send the file content
                while (infile.read(buffer, sizeof(buffer))) {
                    send(client_socket, buffer, infile.gcount(), 0);
                }
                send(client_socket, buffer, infile.gcount(), 0); // Send any remaining bytes
                std::cout << "File downloaded successfully: " << filename << "\n";
                infile.close();
            } else {
                std::cerr << "File not found for download: " << filename << "\n";
                const char* not_found = "File not found";
                send(client_socket, not_found, strlen(not_found), 0);
            }
        } else if (command == "GET_FILE_LIST") {
            std::string file_list;
            for (const auto& entry : std::filesystem::directory_iterator("server")) {
                file_list += entry.path().filename().string() + "\n";
            }
            send(client_socket, file_list.c_str(), file_list.size(), 0);
        } else if (command == "SEARCH_FILE") {
            // Get the filename to search
            read(client_socket, buffer, sizeof(buffer));
            std::string filename(buffer);
            std::string response = "File " + filename + (std::filesystem::exists("server/" + filename) ? " found." : " not found.");
            send(client_socket, response.c_str(), response.size(), 0);
        } else if (command == "DELETE_FILE") {
            // Get the filename to delete
            read(client_socket, buffer, sizeof(buffer));
            std::string filename(buffer);
            if (std::filesystem::remove("server/" + filename)) {
                std::cout << "File deleted successfully: " << filename << "\n";
                const char* success_msg = "File deleted successfully.";
                send(client_socket, success_msg, strlen(success_msg), 0);
            } else {
                std::cerr << "Failed to delete file: " << filename << "\n";
                const char* failure_msg = "File not found for deletion.";
                send(client_socket, failure_msg, strlen(failure_msg), 0);
            }
        }
        memset(buffer, 0, sizeof(buffer)); // Clear the buffer
    }

    close(client_socket);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation error\n";
        return -1;
    }

    // Attach socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::cerr << "Set socket options error\n";
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the port
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed\n";
        return -1;
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed\n";
        return -1;
    }

    std::cout << "Server listening on port " << PORT << "\n";

    // Create the server directory if it doesn't exist
    std::filesystem::create_directory("server");

    while (true) {
        // Accept a client connection
        if ((client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Accept failed\n";
            continue;
        }

        std::cout << "New client connected.\n";
        handleClient(client_socket);
    }

    close(server_fd);
    return 0;
}
