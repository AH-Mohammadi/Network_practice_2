#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080

void handleUpload(int clientSock, const std::string& filename) {
    // Receive file data from the client
    std::ofstream outfile(filename, std::ios::binary);
    if (!outfile) {
        std::string errorMsg = "Server: Unable to create file -> " + filename;
        send(clientSock, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    char buffer[1024];
    int bytesRead;
    while ((bytesRead = read(clientSock, buffer, sizeof(buffer))) > 0) {
        outfile.write(buffer, bytesRead);
    }

    outfile.close();
    std::cout << "Server: File uploaded successfully -> " << filename << "\n";
}

void handleDownload(int clientSock, const std::string& filename) {
    std::ifstream infile(filename, std::ios::binary);
    if (!infile) {
        std::string errorMsg = "Server: File not located -> " + filename;
        send(clientSock, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    // Create a new filename for the download
    std::string downloadFilename = "download_" + filename;
    char buffer[1024];
    while (infile.read(buffer, sizeof(buffer)) || infile.gcount() > 0) {
        send(clientSock, buffer, infile.gcount(), 0);
    }
    infile.close();
    std::cout << "Server: File download transmitted -> " << downloadFilename << "\n";
}

void handleFileList(int clientSock) {
    // Provide the list of files in the server directory (example implementation)
    // Here you would typically generate a file list from the server's directory
    std::string fileList = "file1.txt\nfile2.txt\nfile3.txt\n";
    send(clientSock, fileList.c_str(), fileList.size(), 0);
    std::cout << "Server: File list sent to client\n";
}

void handleClient(int clientSock) {
    char buffer[1024];
    int bytesRead;

    while ((bytesRead = read(clientSock, buffer, sizeof(buffer))) > 0) {
        buffer[bytesRead] = '\0';
        std::string command(buffer);

        if (command == "UPLOAD") {
            read(clientSock, buffer, sizeof(buffer));
            handleUpload(clientSock, std::string(buffer));
        } else if (command == "DOWNLOAD") {
            read(clientSock, buffer, sizeof(buffer));
            handleDownload(clientSock, std::string(buffer));
        } else if (command == "GET_FILE_LIST") {
            handleFileList(clientSock);
        } else {
            std::cerr << "Server: Unknown command received\n";
        }
    }

    close(clientSock);
}

int main() {
    int server_fd, clientSock;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation error\n";
        return -1;
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::cerr << "setsockopt failed\n";
        close(server_fd);
        return -1;
    }

    // Define server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Binding failed\n";
        close(server_fd);
        return -1;
    }

    // Start listening for connections
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listening failed\n";
        close(server_fd);
        return -1;
    }

    std::cout << "Server listening on port " << PORT << "\n";

    while (true) {
        // Accept a new client connection
        if ((clientSock = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
            std::cerr << "Accepting connection failed\n";
            continue;
        }

        std::cout << "New client connected\n";
        handleClient(clientSock);
    }

    close(server_fd);
    return 0;
}
