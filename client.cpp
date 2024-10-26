#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

void uploadFile(int sock, const std::string& filename) {
    send(sock, "UPLOAD", strlen("UPLOAD"), 0);
    send(sock, filename.c_str(), filename.length(), 0);

    std::ifstream infile(filename, std::ios::binary);
    if (!infile) {
        std::cerr << "Unable to open the file: " << filename << "\n";
        return;
    }

    char file_buffer[1024];
    while (infile.read(file_buffer, sizeof(file_buffer))) {
        send(sock, file_buffer, infile.gcount(), 0);
    }
    if (infile.gcount() > 0) {
        send(sock, file_buffer, infile.gcount(), 0);
    }

    std::cout << "File uploaded successfully: " << filename << "\n";
    infile.close();
}

void downloadFile(int sock, const std::string& filename) {
    send(sock, "DOWNLOAD", strlen("DOWNLOAD"), 0);
    send(sock, filename.c_str(), filename.length(), 0);

    std::string newFilename = "downloaded_" + filename;
    std::ofstream outfile(newFilename, std::ios::binary);
    if (!outfile) {
        std::cerr << "Unable to create the file: " << newFilename << "\n";
        return;
    }

    char file_buffer[1024];
    int bytes = 0;
    while ((bytes = read(sock, file_buffer, sizeof(file_buffer))) > 0) {
        outfile.write(file_buffer, bytes);
    }

    std::cout << "File downloaded successfully: " << newFilename << "\n";
    outfile.close();
}

void deleteFile(int sock, const std::string& filename) {
    send(sock, "DELETE_FILE", strlen("DELETE_FILE"), 0);
    send(sock, filename.c_str(), filename.length(), 0);

    char buffer[1024] = {0};
    int bytes_read = read(sock, buffer, sizeof(buffer));
    if (bytes_read > 0) {
        std::cout << "Server response: " << buffer << "\n";
    } else {
        std::cerr << "Failed to receive response for delete request\n";
    }
}

void listFiles(int sock) {
    send(sock, "GET_FILE_LIST", strlen("GET_FILE_LIST"), 0);

    char buffer[1024] = {0};
    int bytes_read = read(sock, buffer, sizeof(buffer));
    if (bytes_read > 0) {
        std::cout << "Files on the server:\n" << buffer << "\n";
    } else {
        std::cerr << "Failed to receive file list\n";
    }
}

void searchFile(int sock, const std::string& filename) {
    send(sock, "SEARCH_FILE", strlen("SEARCH_FILE"), 0);
    send(sock, filename.c_str(), filename.length(), 0);

    char buffer[1024] = {0};
    int bytes_read = read(sock, buffer, sizeof(buffer));
    if (bytes_read > 0) {
        std::cout << "Server response for search: " << buffer << "\n";
    } else {
        std::cerr << "Failed to receive response for search request\n";
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

    // Example usage
