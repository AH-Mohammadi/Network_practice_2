#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>

#define PORT 8080

void showMenu() {
    std::cout << "Menu:\n";
    std::cout << "1. Delete a file\n";
    std::cout << "2. Get file list\n";
    std::cout << "3. Search for a file\n";
    std::cout << "4. Upload a file\n";
    std::cout << "5. Download a file\n";
    std::cout << "6. Exit\n";
}

void uploadFile(int sock) {
    std::string filename;
    std::cout << "Enter the filename to upload: ";
    std::getline(std::cin, filename);

    // Send the command to upload a file
    const char* command = "UPLOAD_FILE";
    send(sock, command, strlen(command), 0);

    // Send the filename
    send(sock, filename.c_str(), filename.length(), 0);

    // Open the file for reading
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file: " << filename << "\n";
        return;
    }

    // Read file and send it to the server
    char buffer[1024];
    while (file.read(buffer, sizeof(buffer))) {
        send(sock, buffer, file.gcount(), 0);
    }
    // Send any remaining bytes
    send(sock, buffer, file.gcount(), 0);
    file.close();

    std::cout << "File uploaded successfully.\n";
}

void downloadFile(int sock) {
    std::string filename;
    std::cout << "Enter the filename to download: ";
    std::getline(std::cin, filename);

    // Send the command to download a file
    const char* command = "DOWNLOAD_FILE";
    send(sock, command, strlen(command), 0);

    // Send the filename
    send(sock, filename.c_str(), filename.length(), 0);

    // Open the file for writing
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error creating file: " << filename << "\n";
        return;
    }

    // Receive file data from the server
    char buffer[1024];
    int bytes_read;
    while ((bytes_read = read(sock, buffer, sizeof(buffer))) > 0) {
        file.write(buffer, bytes_read);
    }
    file.close();

    if (bytes_read < 0) {
        std::cerr << "Error receiving file.\n";
    } else {
        std::cout << "File downloaded successfully.\n";
    }
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Create a socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error\n";
        return -1;
    }

    // Configure the server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/Address not supported\n";
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed\n";
        return -1;
    }

    int choice;
    while (true) {
        showMenu();
        std::cout << "Enter your choice: ";
        std::cin >> choice;
        std::cin.ignore();  // Ignore the newline character left in the buffer

        switch (choice) {
            case 1: {  // Delete a file
                const char* command = "DELETE_FILE";
                send(sock, command, strlen(command), 0);

                std::string filename;
                std::cout << "Enter the filename to delete: ";
                std::getline(std::cin, filename);
                send(sock, filename.c_str(), filename.length(), 0);

                char buffer[1024] = {0};
                int bytes_read = read(sock, buffer, sizeof(buffer));
                if (bytes_read > 0) {
                    std::cout << "Server response: " << buffer << "\n";
                } else {
                    std::cerr << "Failed to receive response\n";
                }
                break;
            }
            case 2: {  // Get file list
                const char* command = "GET_FILE_LIST";
                send(sock, command, strlen(command), 0);

                char buffer[1024] = {0};
                int bytes_read = read(sock, buffer, sizeof(buffer));
                if (bytes_read > 0) {
                    std::cout << "File list received from server:\n" << buffer;
                } else {
                    std::cerr << "Failed to receive file list\n";
                }
                break;
            }
            case 3: {  // Search for a file
                const char* command = "SEARCH_FILE";
                send(sock, command, strlen(command), 0);

                std::string filename;
                std::cout << "Enter the filename to search: ";
                std::getline(std::cin, filename);
                send(sock, filename.c_str(), filename.length(), 0);

                char buffer[1024] = {0};
                int bytes_read = read(sock, buffer, sizeof(buffer));
                if (bytes_read > 0) {
                    std::cout << "Server response: " << buffer << "\n";
                } else {
                    std::cerr << "Failed to receive response\n";
                }
                break;
            }
            case 4: {  // Upload a file
                uploadFile(sock);
                break;
            }
            case 5: {  // Download a file
                downloadFile(sock);
                break;
            }
            case 6: {  // Exit
                std::cout << "Exiting...\n";
                close(sock);
                return 0;
            }
            default:
                std::cout << "Invalid choice, please try again.\n";
        }
    }

    close(sock);
    return 0;
}
