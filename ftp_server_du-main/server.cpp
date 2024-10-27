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
    char downloadFilleName[1024] = {0};
    read(SOCKET, downloadFilleName, 1024);

    std::ifstream infile(downloadFilleName, std::ios::binary);
    if (!infile) {
        std::cerr << "NOT UPLOAD 404: " << downloadFilleName << "\n";
        return;
    }

    char file_buffer[1024];
    int bytes = 0;
    while ((bytes = infile.readsome(file_buffer, sizeof(file_buffer))) > 0) {
        send(SOCKET, file_buffer, bytes, 0);
    }

    std::cout << "File successfully downloaded\n";
    infile.close();
}

void serverUpload(int SOCKET) {
    char uploadFileName[1024] = {0};
    read(SOCKET, uploadFileName, 1024);

    std::ofstream outfile(uploadFileName, std::ios::binary);
    if (!outfile) {
        std::cerr << "file didn't create!: " << uploadFileName << "\n";
        return;
    }

    char file_buffer[1024];
    int bytes = 0;
    while ((bytes = read(SOCKET, file_buffer, sizeof(file_buffer))) > 0) {
        outfile.write(file_buffer, bytes);
    }

    std::cout << "upload file successfully done " << uploadFileName << "\n";
    outfile.close();
}



void serverSearchFile(int SOCKET) {
    char searchFilleName[1024] = {0};
    read(SOCKET, searchFilleName, 1024);

    std::ifstream infile(searchFilleName);
    const char* response = infile ? "File exsits:  FOUND!";
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
        std::cout << "file List: " << fileList;
    } else {
        std::cerr << "directory Not opened: " << pathDir << "\n";
    }
}

void serverDeleteFile(int SOCKET) {
    char deleteFileName[1024] = {0};
    read(SOCKET, deleteFileName, 1024);

    if (remove(deleteFileName) == 0) {
        const char* resp = "file deleted now.";
        send(SOCKET, resp, strlen(resp), 0);
        std::cout << "the name of file was deleted is: " << deleteFileName << "\n";
    } else {
        const char* resp = "Failed delete file.";
        send(SOCKET, resp, strlen(resp), 0);
        std::cerr << "deleting file: " << deleteFileName << "\n";
    }
}

int main() {
    int server_fd, SOCKET;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "socket connection fialed\n";
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::cerr << "socketopt failed\n";
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "BIND ERROr\n";
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen ERROR\n";
        return -1;
    }

    while (true) {
        std::cout << "Connnection waiting .\n";
        if ((SOCKET = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Accept failed\n";
            return -1;
        }

        char command[1024] = {0};
        read(SOCKET, command, 1024);

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
