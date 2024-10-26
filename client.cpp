#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080

void uploadFile(int sock) {
    char filename[1024];
    std::cout << "Enter the filename to upload: ";
    std::cin >> filename;

    // Send upload request
    send(sock, "UPLOAD", strlen("UPLOAD"), 0);
    send(sock, filename, strlen(filename), 0);

    // Open the file to be uploaded
    std::ifstream infile(filename, std::ios::binary);
    if (!infile) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return;
    }

    // Read the file and send it to the server
    char file_buffer[1024];
    while (!infile.eof()) {
        infile.read(file_buffer, sizeof(file_buffer));
        int bytes_read = infile.gcount();
        if (bytes_read > 0) {
            send(sock, file_buffer, bytes_read, 0);
        }
    }

    std::cout << "File uploaded successfully as " << filename << "\n";
    infile.close();
}

void downloadFile(int sock) {
    char filename[1024];
    std::cout << "Enter the filename to download: ";
    std::cin >> filename;

    // Send download request
    send(sock, "DOWNLOAD", strlen("DOWNLOAD"), 0);
    send(sock, filename, strlen(filename), 0);

    // Prepare the new filename for download
    std::string newFilename = "downloaded_" + std::string(filename);

    // Open a file to save the downloaded content
    std::ofstream outfile(newFilename, std::ios::binary);
    if (!outfile) {
        std::cerr << "Failed to create file: " << newFilename << "\n";
        return;
    }

    // Receive data from the server and write it to the file
    char file_buffer[1024];
    int bytes = 0;
    while ((bytes = read(sock, file_buffer, sizeof(file_buffer))) > 0) {
        outfile.write(file_buffer, bytes);
    }

    std::cout << "File downloaded successfully as " << newFilename << "\n";
    outfile.close();
}

void listFiles(int sock) {
    // Send request to get file list
    send(sock, "GET_FILE_LIST", strlen("GET_FILE_LIST"), 0);
    
    char fileList[4096] = {0}; // Buffer for file list
    read(sock, fileList, sizeof(fileList));
    std::cout << "Files on server:\n" << fileList;
}

void searchFile(int sock) {
    char filename[1024];
    std::cout << "Enter the filename to search: ";
    std::cin >> filename;

    // Send search request
    send(sock, "SEARCH_FILE", strlen("SEARCH_FILE"), 0);
    send(sock, filename, strlen(filename), 0);

    char response[1024];
    read(sock, response, sizeof(response));
    std::cout << response << "\n";
}

void deleteFile(int sock) {
    char filename[1024];
    std::cout << "Enter the filename to delete: ";
    std::cin >> filename;

    // Send delete request
    send(sock, "DELETE_FILE", strlen("DELETE_FILE"), 0);
    send(sock, filename, strlen(filename), 0);

    char response[1024];
    read(sock, response, sizeof(response));
    std::cout << response << "\n";
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
        std::cout << "\nMenu:\n";
        std::cout << "1. Upload File\n";
        std::cout << "2. Download File\n";
        std::cout << "3. List Files\n";
        std::cout << "4. Search File\n";
        std::cout << "5. Delete File\n";
        std::cout << "6. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
            case 1:
                uploadFile(sock);
                break;
            case 2:
                downloadFile(sock);
                break;
            case 3:
                listFiles(sock);
                break;
            case 4:
                searchFile(sock);
                break;
            case 5:
                deleteFile(sock);
                break;
            case 6:
                std::cout << "Exiting...\n";
                close(sock);
                return 0;
            default:
                std::cout << "Invalid choice. Please try again.\n";
        }
    }

    // Close the socket
    close(sock);
    return 0;
}
