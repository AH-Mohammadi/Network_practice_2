#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <dirent.h>

#define PORT 8080

void handleUpload(int clientSock, const std::string& filename) {
    std::ofstream outfile(filename, std::ios::binary);
    if (!outfile) {
        std::cerr << "Server: Issue creating file for upload: " << filename << "\n";
        return;
    }

    char buffer[1024];
    int bytes;
    while ((bytes = read(clientSock, buffer, sizeof(buffer))) > 0) {
        outfile.write(buffer, bytes);
    }
    outfile.close();
    std::cout << "Server: File upload completed -> " << filename << "\n";
}

void handleDownload(int clientSock, const std::string& filename) {
    std::ifstream infile(filename, std::ios::binary);
    if (!infile) {
        std::string errorMsg = "Server: File not located -> " + filename;
        send(clientSock, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    char buffer[1024];
    while (infile.read(buffer, sizeof(buffer)) || infile.gcount() > 0) {
        send(clientSock, buffer, infile.gcount(), 0);
    }
    infile.close();
    std::cout << "Server: File download transmitted -> " << filename << "\n";
}

void handleDelete(int clientSock, const std::string& filename) {
    if (remove(filename.c_str()) == 0) {
        std::string successMsg = "Server: File successfully removed -> " + filename;
        send(clientSock, successMsg.c_str(), successMsg.size(), 0);
        std::cout << "Server: File deleted -> " << filename << "\n";
    } else {
        std::string errorMsg = "Server: Unable to delete -> " + filename;
        send(clientSock, errorMsg.c_str(), errorMsg.size(), 0);
    }
}

void handleList(int clientSock) {
    DIR* dir;
    struct dirent* entry;
    std::stringstream fileList;

    if ((dir = opendir(".")) != nullptr) {
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_REG) {
                fileList << entry->d_name << "\n";
            }
        }
        closedir(dir);
    } else {
        fileList << "Server: Issue accessing directory\n";
    }

    std::string listStr = fileList.str();
    send(clientSock, listStr.c_str(), listStr.size(), 0);
    std::cout << "Server: File listing sent to client\n";
}

void handleSearch(int clientSock, const std::string& filename) {
    if (access(filename.c_str(), F_OK) != -1) {
        std::string foundMsg = "Server: File located -> " + filename;
        send(clientSock, foundMsg.c_str(), foundMsg.size(), 0);
    } else {
        std::string notFoundMsg = "Server: File not found -> " + filename;
        send(clientSock, notFoundMsg.c_str(), notFoundMsg.size(), 0);
    }
    std::cout << "Server: Search completed for -> " << filename << "\n";
}

void processClientRequest(int clientSock) {
    char command[1024] = {0};
    read(clientSock, command, sizeof(command));
    std::string commandStr(command);

    if (commandStr == "UPLOAD") {
        char filename[1024] = {0};
        read(clientSock, filename, sizeof(filename));
        handleUpload(clientSock, filename);
    } else if (commandStr == "DOWNLOAD") {
        char filename[1024] = {0};
        read(clientSock, filename, sizeof(filename));
        handleDownload(clientSock, filename);
    } else if (commandStr == "DELETE_FILE") {
        char filename[1024] = {0};
        read(clientSock, filename, sizeof(filename));
        handleDelete(clientSock, filename);
    } else if (commandStr == "GET_FILE_LIST") {
        handleList(clientSock);
    } else if (commandStr == "SEARCH_FILE") {
        char filename[1024] = {0};
        read(clientSock, filename, sizeof(filename));
        handleSearch(clientSock, filename);
    } else {
        std::cerr << "Server: Unknown command received\n";
    }
}

int main() {
    int serverSock, clientSock;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((serverSock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Server: Failed to initialize socket\n";
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(serverSock, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Server: Socket binding issue\n";
        return -1;
    }

    if (listen(serverSock, 3) < 0) {
        std::cerr << "Server: Listen failure\n";
        return -1;
    }

    std::cout << "Server: Awaiting client connections on port " << PORT << "\n";

    while (true) {
        if ((clientSock = accept(serverSock, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Server: Client acceptance issue\n";
            continue;
        }

        std::cout << "Server: Client connected\n";
        processClientRequest(clientSock);
        close(clientSock);
        std::cout << "Server: Client disconnected\n";
    }

    close(serverSock);
    return 0;
}
