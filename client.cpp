#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <filesystem>

#define PORT 8080

void uploadFile(int sock) {
    std::string filename;
    std::cout << "Enter the filename to upload: ";
    std::getline(std::cin, filename);

    // Check if the file exists in the "server" folder
    std::filesystem::path filepath = "server/" + filename;
    if (!std::filesystem::exists(filepath)) {
        std::cerr << "File does not exist in the server folder.\n";
        return;
    }

    // Send the UPLOAD command
    const char* command = "UPLOAD";
    send(sock, command, strlen(command), 0);

    // Send the filename
    send(sock, filename.c_str(), filename.length(), 0);

    // Open the file and send its contents
    std::ifstream infile(filepath, std::ios::binary);
    char buffer[1024];
    while (infile.read(buffer, sizeof(buffer))) {
        send(sock, buffer, infile.gcount(), 0);
    }
    send(sock, buffer, infile.gcount(), 0); // Send any remaining bytes
    std::cout << "File uploaded successfully.\n";
    infile.close();
}

void downloadFile(int sock) {
    std::string filename;
    std::cout << "Enter the filename to download: ";
    std::getline(std::cin, filename);

    // Send the DOWNLOAD command
    const char* command = "DOWNLOAD";
    send(sock, command, strlen(command), 0);

    // Send the filename
    send(sock, filename.c_str(), filename.length(), 0);

    // Open the file to write the received data
    std::ofstream outfile("server/" + filename, std::ios::binary);
    char buffer[1024];
    int bytes_read;

    // Receive the file data
    while ((bytes_read = read(sock, buffer, sizeof(buffer))) > 0) {
        outfile.write(buffer, bytes_read);
    }
    std::cout << "File downloaded successfully.\n";
    outfile.close();
}

void getFileList(int sock) {
    const char* command = "GET_FILE_LIST";
    send(sock, command, strlen(command), 0);

    char buffer[1024] = {0};
    int bytes_read = read(sock, buffer, sizeof(buffer));
    if (bytes_read > 0) {
        std::cout << "File list received from server:\n" << buffer;
    } else {
        std::cerr << "Failed to receive file list\n";
    }
}

void searchFile(int sock) {
    std::string filename;
    std::cout << "Enter the filename to search: ";
    std::getline(std::cin, filename);

    const char* command = "SEARCH_FILE";
    send(sock, command, strlen(command), 0);
    send(sock, filename.c_str(), filename.length(), 0);

    char buffer[1024] = {0};
    int bytes_read = read(sock, buffer, sizeof(buffer));
    if (bytes_read > 0) {
        std::cout << "Server response: " << buffer << "\n";
    } else {
        std::cerr << "Failed to receive response\n";
    }
}

void deleteFile(int sock) {
    std::string filename;
    std::cout << "Enter the filename to delete: ";
    std::getline(std::cin, filename);

    const char* command = "DELETE_FILE";
    send(sock, command, strlen(command), 0);
    send(sock, filename.c_str(), filename.length(), 0);

    char buffer[1024] = {0};
    int bytes_read = read(sock, buffer, sizeof(buffer));
    if (bytes_read > 0) {
        std::cout << "Server response: " << buffer << "\n";
    } else {
        std::cerr << "Failed to receive response\n";
    }
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error\n";
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/Address not supported\n";
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed\n";
        return -1;
    }

    int choice;
    do {
        std::cout << "Choose an operation:\n";
        std::cout << "1. Upload File\n";
        std::cout << "2. Download File\n";
        std::cout << "3. Get File List\n";
        std::cout << "4. Search File\n";
        std::cout << "5. Delete File\n";
        std::cout << "0. Exit\n";
        std::cin >> choice;
        std::cin.ignore(); // Clear newline from the input buffer

        switch (choice) {
            case 1:
                uploadFile(sock);
                break;
            case 2:
                downloadFile(sock);
                break;
            case 3:
                getFileList(sock);
                break;
            case 4:
                searchFile(sock);
                break;
            case 5:
                deleteFile(sock);
                break;
            case 0:
                std::cout << "Exiting...\n";
                break;
            default:
                std::cerr << "Invalid choice. Please try again.\n";
        }
    } while (choice != 0);

    close(sock);
    return 0;
}
