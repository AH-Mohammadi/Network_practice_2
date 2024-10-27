#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Create a socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "There is a creation ERROR";
        return -1;
    }

    // Configure the server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "INVALID ADDRESS\n";
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Failed Connection \n";
        return -1;
    }

    // Send the command to search for a file
    const char* command = "SEARCH_FILE";
    send(sock, command, strlen(command), 0);

    // Get the filename to search for
    std::string filename;
    std::cout << "file name is entered: ";
    std::getline(std::cin, filename);

    // Send the filename to the server
    send(sock, filename.c_str(), filename.length(), 0);

    // Receive the response from the server
    char buffer[1024] = {0};
    int bytes_read = read(sock, buffer, sizeof(buffer));
    if (bytes_read > 0) {
        std::cout << "Server response is : " << buffer << "\n";
    } else {
        std::cerr << "there is error on reciving answer\n";
    }

    // Close the socket
    close(sock);
    return 0;
}
