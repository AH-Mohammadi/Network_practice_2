#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

void uploadFile(int sock, const std::string& filename) {
    // Send upload request
    send(sock, "UPLOAD", strlen("UPLOAD"), 0);

    // Send the filename to the server
    send(sock, filename.c_str(), filename.length(), 0);

    // Open the file to be uploaded
    std::ifstream infile(filename, std::ios::binary);
    if (!infile) {
        std::cerr << "Unable to open the file: " << filename << "\n";
        return;
    }

    // Read the file and send it to the server
    char file_buffer[1024];
    while (infile.read(file_buffer, sizeof(file_buffer))) {
        send(sock, file_buffer, infile.gcount(), 0);
    }
    // Send any remaining bytes
    if (infile.gcount() > 0) {
        send(sock, file_buffer, infile.gcount(), 0);
    }

    std::cout << "File uploaded successfully: " << filename << "\n";
    infile.close();
}

void downloadFile(int sock, const std::string& filename) {
    // Send download request
    send(sock, "DOWNLOAD", strlen("DOWNLOAD"), 0);

    // Send the filename to download
    send(sock, filename.c_str(), filename.length(), 0);

    // Prepare the new filename for download
    std::string newFilename = "downloaded_" + filename;

    // Open a file to save the downloaded content
    std::ofstream outfile(newFilename, std::ios::binary);
    if (!outfile) {
        std::cerr << "Unable to create the file: " << newFilename << "\n";
        return;
    }

    // Receive data from the server and write it to the file
    char file_buffer[1024];
    int bytes = 0;
    while ((bytes = read(sock, file_buffer, sizeof(file_buffer))) > 0) {
        outfile.write(file_buffer, bytes);
    }

    std::cout << "File downloaded successfully: " << newFilename << "\n";
    outfile.close();
}

void deleteFile(int sock, const std::string& filename) {
    // Send delete request
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
    // Send list request
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
    // Send search request
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

    // Example operations
    uploadFile(sock, "example.txt");  // Upload a file
    downloadFile(sock, "example.txt"); // Download the file
    deleteFile(sock, "example.txt");   // Delete the file
    listFiles(sock);                    // List files
    searchFile(sock, "example.txt");    // Search for a file

    // Close the socket
    close(sock);
    return 0;
}
