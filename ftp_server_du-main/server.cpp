#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <dirent.h>
#include <sys/stat.h>

#define PORT 8080

void serverDownload(int SOCKET) {
    char downloadFileName[1024] = {0};
    read(SOCKET, downloadFileName, sizeof(downloadFileName));

    std::ifstream infile(downloadFileName, std::ios::binary);
    if (!infile) {
        std::cerr << "File not found (404): " << downloadFileName << "\n";
        const char* errorMsg = "File not found";
        send(SOCKET, errorMsg, strlen(errorMsg), 0);
        return;
    }

    char file_buffer[1024];
    int bytes = 0;
    while ((bytes = infile.readsome(file_buffer, sizeof(file_buffer))) > 0) {
        send(SOCKET, file_buffer, bytes, 0);
    }

    std::cout << "File successfully downloaded: " << downloadFileName << "\n";
    infile.close();
}

void serverUpload(int SOCKET) {
    char uploadFileName[1024] = {0};
    read(SOCKET, uploadFileName, sizeof(uploadFileName));

    std::ofstream outfile(uploadFileName, std::ios::binary);
    if (!outfile) {
        std::cerr << "Failed to create file: " << uploadFileName << "\n";
        const char* errorMsg = "Failed to create file";
        send(SOCKET, errorMsg, strlen(errorMsg), 0);
        return;
    }

    char file_buffer[1024];
    int bytes = 0;
    while ((bytes = read(SOCKET, file_buffer, sizeof(file_buffer))) > 0) {
        outfile.write(file_buffer, bytes);
    }

    std::cout << "Upload successful: " << uploadFileName << "\n";
    outfile.close();
}

void serverSearchFile(int SOCKET) {
    char searchFileName[1024] = {0};
    read(SOCKET, searchFileName, sizeof(searchFileName));

    std::ifstream infile(searchFileName);
    const char* response = infile ? "File exists: FOUND!" : "File does not exist.";
    send(SOCKET, response, strlen(response), 0);
    infile.close();
}

void serverFileList(int SOCKET) {
    const char* pathDir = ".";
    DIR* dir;
    struct dirent* ent;

    if ((dir = opendir(pathDir)) != nullptr) {
        std::string fileList;

        while ((ent = readdir(dir)) != nullptr) {
            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                fileList += ent->d_name;
                fileList += "\n";
            }
        }

        closedir(dir);
        send(SOCKET, fileList.c_str(), fileList.size(), 0);
        std::cout << "File list sent: " << fileList;
    } else {
        std::cerr << "Failed to open directory: " << pathDir << "\n";
    }
}

void serverDeleteFile(int SOCKET) {
    char deleteFileName[1024] = {0};
    read(SOCKET, deleteFileName, sizeof(deleteFileName));

    if (remove(deleteFileName) == 0) {
        const char* resp = "File deleted successfully.";
        send(SOCKET, resp, strlen(resp), 0);
        std::cout << "Deleted file: " << deleteFileName << "\n";
    } else {
        const char* resp = "Failed to delete file.";
        send(SOCKET, resp, strlen(resp), 0);
        std::cerr << "Error deleting file: " << deleteFileName << "\n";
    }
}

int main() {
    int server_fd, SOCKET;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation failed\n";
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::cerr << "Set socket options failed\n";
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind error\n";
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen error\n";
        return -1;
    }

    while (true) {
        std::cout << "Waiting for connections...\n";
        if ((SOCKET = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Accept failed\n";
            continue;  // Use continue instead of return to keep the server running
        }

        char command[1024] = {0};
        read(SOCKET, command, sizeof(command));

        if (strcmp(command, "UPLOAD") == 0) {
            serverUpload(SOCKET);
        } else if (strcmp(command, "DOWNLOAD") == 0) {
            serverDownload(SOCKET);
        } else if (strcmp(command, "GET_FILE_LIST") == 0) {
            serverFileList(SOCKET);
        } else if (strcmp(command, "SEARCH_FILE") == 0) {
            serverSearchFile(SOCKET);
        } else if (strcmp(command, "DELETE_FILE") == 0) {
            serverDeleteFile(SOCKET);
        }

        close(SOCKET);
    }

    return 0;
}
