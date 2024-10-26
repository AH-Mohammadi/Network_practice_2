#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define SERVER_FOLDER "server"

// Function to create server folder if it doesn't exist
void createServerFolder() {
    struct stat st;
    if (stat(SERVER_FOLDER, &st) != 0) {
        mkdir(SERVER_FOLDER, 0755); // Create the server folder
    }
}

// Function to get a list of files in the server folder
std::string getFileList() {
    std::string fileList;
    struct dirent *entry;
    DIR *dp = opendir(SERVER_FOLDER);

    if (dp == nullptr) {
        perror("opendir");
        return "Error: Unable to open server directory.\n";
    }

    while ((entry = readdir(dp))) {
        // Ignore the current and parent directory entries
        if (entry->d_name[0] != '.') {
            fileList += entry->d_name;
            fileList += "\n";
        }
    }
    closedir(dp);
    return fileList;
}

// Function to handle client requests
void handleClient(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};

    while (true) {
        int bytes_read = read(client_socket, buffer, sizeof(buffer));
        if (bytes_read <= 0) {
            std::cerr << "Client disconnected or error occurred.\n";
            break;
        }

        buffer[bytes_read] = '\0';
        std::string command(buffer);

        if (command == "UPLOAD_FILE") {
            // Get the filename
            read(client_socket, buffer, sizeof(buffer));
            std::string filename(buffer);
            
            // Open the file to write
            std::ofstream file(SERVER_FOLDER + "/" + filename, std::ios::binary);
            if (!file) {
                std::cerr << "Error creating file: " << filename << "\n";
                continue;
            }

            // Receive file data
            while ((bytes_read = read(client_socket, buffer, sizeof(buffer))) > 0) {
                file.write(buffer, bytes_read);
            }
            file.close();
            std::cout << "File uploaded: " << filename << "\n";
            const char* response = "File uploaded successfully.";
            send(client_socket, response, strlen(response), 0);

        } else if (command == "DOWNLOAD_FILE") {
            // Get the filename
            read(client_socket, buffer, sizeof(buffer));
            std::string filename(buffer);
            std::ifstream file(SERVER_FOLDER + "/" + filename, std::ios::binary);
            if (!file) {
                std::cerr << "Error opening file: " << filename << "\n";
                const char* response = "Error: File not found.";
                send(client_socket, response, strlen(response), 0);
                continue;
            }

            // Send the file data
            while (file.read(buffer, sizeof(buffer))) {
                send(client_socket, buffer, file.gcount(), 0);
            }
            file.close();
            std::cout << "File downloaded: " << filename << "\n";

        } else if (command == "DELETE_FILE") {
            // Get the filename
            read(client_socket, buffer, sizeof(buffer));
            std::string filename(buffer);
            if (remove((SERVER_FOLDER + "/" + filename).c_str()) == 0) {
                std::cout << "File deleted: " << filename << "\n";
                const char* response = "File deleted successfully.";
                send(client_socket, response, strlen(response), 0);
            } else {
                std::cerr << "Error deleting file: " << filename << "\n";
                const char* response = "Error: File not found.";
                send(client_socket, response, strlen(response), 0);
            }

        } else if (command == "GET_FILE_LIST") {
            std::string response = getFileList();
            send(client_socket, response.c_str(), response.length(), 0);
            std::cout << "Sent file list to client.\n";

        } else if (command == "SEARCH_FILE") {
            // Get the filename to search for
            read(client_socket, buffer, sizeof(buffer));
            std::string filename(buffer);
            std::ifstream file(SERVER_FOLDER + "/" + filename);
            if (file) {
                const char* response = "File found.";
                send(client_socket, response, strlen(response), 0);
            } else {
                const char* response = "File not found.";
                send(client_socket, response, strlen(response), 0);
            }

        } else {
            std::cerr << "Unknown command: " << command << "\n";
        }
    }

    close(client_socket);
}

int main() {
    createServerFolder(); // Create server folder if it does not exist

    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation failed\n";
        return -1;
    }

    // Attach socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "Set socket options failed\n";
        return -1;
    }

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

    std::cout << "Server is listening on port " << PORT << "...\n";

    while (true) {
        // Accept a connection from a client
        if ((client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Accept failed\n";
            continue;
        }

        std::cout << "Client connected.\n";
        handleClient(client_socket);
    }

    close(server_fd);
    return 0;
}
