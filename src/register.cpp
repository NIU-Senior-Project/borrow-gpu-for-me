#include "register.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>

// Socket 相關標頭檔
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "gpu.h"
#include "auth.h"

// 發送註冊請求到 Node Manager
bool register_node(const std::string& manager_ip, int manager_port, const std::string& node_ip, const std::string& gpu_model) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "[ERROR] Failed to create socket.\n";
        return false;
    }

    sockaddr_in manager_addr{};
    manager_addr.sin_family = AF_INET;
    manager_addr.sin_port = htons(manager_port);

    if (inet_pton(AF_INET, manager_ip.c_str(), &manager_addr.sin_addr) <= 0) {
        std::cerr << "[ERROR] Invalid Node Manager IP address.\n";
        close(sock);
        return false;
    }

    if (connect(sock, reinterpret_cast<sockaddr*>(&manager_addr), sizeof(manager_addr)) < 0) {
        std::cerr << "[ERROR] Failed to connect to Node Manager at " << manager_ip << ":" << manager_port << "\n";
        close(sock);
        return false;
    }

    int gpu_memory_mb = detect_gpu_memory_mb();

    // 歸屬與價格從環境變數讀取（與 AUTH_TOKEN 一致的做法）：
    //   OWNER     公開顯示名稱
    //   OWNER_KEY 擁有權秘密（之後改價/註銷須帶相同值）
    //   PRICE     上架價格（每小時）
    const char* ownerEnv = std::getenv("OWNER");
    const char* ownerKeyEnv = std::getenv("OWNER_KEY");
    const char* priceEnv = std::getenv("PRICE");
    const std::string owner = ownerEnv ? ownerEnv : "";
    const std::string owner_key = ownerKeyEnv ? ownerKeyEnv : "";
    double price = 0.0;
    if (priceEnv) { try { price = std::stod(priceEnv); } catch (...) { price = 0.0; } }

    // 建立 JSON 格式的 Body
    std::ostringstream body;
    body << "{\"ip\": \"" << node_ip
         << "\", \"gpu_model\": \"" << gpu_model
         << "\", \"gpu_memory_mb\": " << gpu_memory_mb
         << ", \"owner\": \"" << owner
         << "\", \"owner_key\": \"" << owner_key
         << "\", \"price\": " << price
         << "}";
    std::string body_str = body.str();

    // 建構 HTTP POST 請求
    std::ostringstream request;
    request << "POST /register HTTP/1.1\r\n"
            << "Host: " << manager_ip << ":" << manager_port << "\r\n"
            << "Content-Type: application/json\r\n"
            << "Content-Length: " << body_str.size() << "\r\n"
            << auth_header_line()
            << "Connection: close\r\n\r\n"
            << body_str;

    std::string req_str = request.str();

    // 發送請求
    if (send(sock, req_str.data(), req_str.size(), 0) != static_cast<ssize_t>(req_str.size())) {
        std::cerr << "[ERROR] Failed to send HTTP request.\n";
        close(sock);
        return false;
    }

    // 讀取 Response
    char buffer[4096];
    std::string response;
    ssize_t bytesRead;
    while ((bytesRead = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesRead] = '\0';
        response.append(buffer);
    }

    close(sock);

    // 檢查 HTTP Response Code 是否為 200 OK
    if (response.find("200 OK") != std::string::npos) {
        std::cout << "[INFO] Successfully registered GPU resource to Manager!\n";
        return true;
    } else {
        std::cerr << "[ERROR] Registration failed. Server response:\n" << response << "\n";
        return false;
    }
}
