#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080

// Upload a file to the server
void uploadFile(int sock, const std::string& filename) {
    send(sock, "UPLOAD", strlen("UPLOAD"), 0);
    send(sock, filename.c_str(), filename.size(), 0);

    std::ifstream infile(filename, std::ios::binary);
    if (!infile) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return;
    }

    char buffer[1024];
    while (infile.read(buffer, sizeof(buffer)) || infile.gcount() > 0) {
        send(sock, buffer, infile.gcount(), 0);
    }
    std::cout << "File uploaded successfully as " << filename << "\n";
}

// Download a file from the server
void downloadFile(int sock, const std::string& filename) {
    send(sock, "DOWNLOAD", strlen("DOWNLOAD"), 0);
    send(sock, filename.c_str(), filename.size(), 0);

    std::string newFilename = "downloaded_" + filename;
    std::ofstream outfile(newFilename, std::ios::binary);
    if (!outfile) {
        std::cerr << "Failed to create file: " << newFilename << "\n";
        return;
    }

    char buffer[1024];
    int bytes = 0;
    while ((bytes = read(sock, buffer, sizeof(buffer))) > 0) {
        outfile.write(buffer, bytes);
    }
    std::cout << "File downloaded successfully as " << newFilename << "\n";
}

// Delete a file on the server
void deleteFile(int sock) {
    send(sock, "DELETE_FILE", strlen("DELETE_FILE"), 0);
    std::string filename;
    std::cout << "Enter the filename to delete: ";
    std::getline(std::cin, filename);

    send(sock, filename.c_str(), filename.size(), 0);
    char response[1024] = {0};
    int bytesReceived = read(sock, response, sizeof(response));
    std::cout << "Server response: " << response << "\n";
}

// List files on the server
void listFiles(int sock) {
    send(sock, "GET_FILE_LIST", strlen("GET_FILE_LIST"), 0);
    char buffer[1024] = {0};
    int bytesRead = read(sock, buffer, sizeof(buffer));
    std::cout << "File list received from server:\n" << buffer;
}

// Search for a file on the server
void searchFile(int sock) {
    send(sock, "SEARCH_FILE", strlen("SEARCH_FILE"), 0);
    std::string filename;
    std::cout << "Enter the filename to search: ";
    std::getline(std::cin, filename);

    send(sock, filename.c_str(), filename.size(), 0);
    char buffer[1024] = {0};
    int bytesRead = read(sock, buffer, sizeof(buffer));
    std::cout << "Server response: " << buffer << "\n";
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
    std::string filename;
    do {
        std::cout << "\nChoose an option:\n1. Upload file\n2. Download file\n3. Delete file\n4. List files\n5. Search file\n0. Exit\n";
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
            case 1:
                std::cout << "Enter filename to upload: ";
                std::getline(std::cin, filename);
                uploadFile(sock, filename);
                break;
            case 2:
                std::cout << "Enter filename to download: ";
                std::getline(std::cin, filename);
                downloadFile(sock, filename);
                break;
            case 3:
                deleteFile(sock);
                break;
            case 4:
                listFiles(sock);
                break;
            case 5:
                searchFile(sock);
                break;
            case 0:
                std::cout << "Exiting client.\n";
                break;
            default:
                std::cout << "Invalid choice.\n";
        }
    } while (choice != 0);

    close(sock);
    return 0;
}
