#include "job.h"

#include <iostream>
#include <string>
#include <sstream>

// Socket 相關標頭檔 (給監聽 Server 用)
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

void start_job_listener(int port) {
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        std::cerr << "Failed to create node listener socket.\n";
        return;
    }

    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(serverFd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        std::cerr << "Failed to bind on port " << port << ".\n";
        close(serverFd);
        return;
    }

    if (listen(serverFd, 5) < 0) {
        std::cerr << "Failed to listen.\n";
        close(serverFd);
        return;
    }

    std::cout << "[INFO] Node is now listening for incoming jobs on port " << port << "...\n";

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientFd = accept(serverFd, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
        if (clientFd < 0) continue;

        std::string request;
        char buffer[4096];
        ssize_t bytesRead;
        while ((bytesRead = recv(clientFd, buffer, sizeof(buffer), 0)) > 0) {
            request.append(buffer, bytesRead);
            if (request.find("\r\n\r\n") != std::string::npos) break; // 讀完 Header 就先停
        }

        auto headerEnd = request.find("\r\n\r\n");
        if (headerEnd != std::string::npos) {
            std::string body = request.substr(headerEnd + 4);
            
            // 這裡簡單印出收到的任務內容，你可以後續在這裡接上 Docker 執行的邏輯
            std::cout << "\n[NEW JOB RECEIVED]\n";
            std::cout << body << "\n";
            std::cout << "------------------\n";

            // 回傳成功訊息給 Manager
            std::string respBody = "Job accepted and is running on Node.\n";
            std::ostringstream response;
            response << "HTTP/1.1 200 OK\r\n"
                     << "Content-Type: text/plain\r\n"
                     << "Content-Length: " << respBody.size() << "\r\n"
                     << "Connection: close\r\n\r\n"
                     << respBody;
                     
            send(clientFd, response.str().data(), response.str().size(), 0);
        }
        close(clientFd);
    }
}
