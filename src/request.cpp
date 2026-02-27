#include <functional>
#include <iostream>
#include <sstream>
#include <string>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

namespace {

std::string build_node_id(const std::string& gpu_model,
                          const std::string& ip) {
    const auto hash_value = std::hash<std::string>{}(gpu_model + "@" + ip);
    std::ostringstream os;
    os << "node-" << std::hex << hash_value;
    return os.str();
}

}  // namespace

int register_gpu(std::string gpu_model, std::string ip) {
    if (gpu_model.empty() || ip.empty()) {
        std::cerr << "register_gpu failed: gpu_model/ip must not be empty"
                  << std::endl;
        return -1;
    }

    const std::string node_id = build_node_id(gpu_model, ip);
    const int gpu_memory_mb = 0;

    std::ostringstream payload;
    payload << "RegisterNodeRequest {\n"
            << "  node_id: \"" << node_id << "\"\n"
            << "  node_address: \"" << ip << "\"\n"
            << "  gpu_model: \"" << gpu_model << "\"\n"
            << "  gpu_memory_mb: " << gpu_memory_mb << "\n"
            << "}";

    std::cout << payload.str() << std::endl;
    return 0;
}

int view_online_gpus(std::string manager_ip) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Failed to create socket.\n";
        return -1;
    }

    sockaddr_in manager_addr{};
    manager_addr.sin_family = AF_INET;
    manager_addr.sin_port = htons(8080); // 假設 Manager 監聽 8080 port

    if (inet_pton(AF_INET, manager_ip.c_str(), &manager_addr.sin_addr) <= 0) {
        std::cerr << "Invalid Manager IP address.\n";
        close(sock);
        return -1;
    }

    if (connect(sock, reinterpret_cast<sockaddr*>(&manager_addr), sizeof(manager_addr)) < 0) {
        std::cerr << "Failed to connect to Manager at " << manager_ip << ":8080\n";
        close(sock);
        return -1;
    }

    // 建構 HTTP GET 請求
    std::ostringstream request;
    request << "GET /nodes HTTP/1.1\r\n"
            << "Host: " << manager_ip << ":8080\r\n"
            << "Connection: close\r\n\r\n";

    std::string req_str = request.str();

    // 發送請求
    if (send(sock, req_str.data(), req_str.size(), 0) != static_cast<ssize_t>(req_str.size())) {
        std::cerr << "Failed to send HTTP request.\n";
        close(sock);
        return -1;
    }

    // 接收 Response
    std::string response;
    char buffer[4096];
    ssize_t bytesRead;
    while ((bytesRead = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesRead] = '\0';
        response.append(buffer);
    }
    close(sock);

    // 解析 HTTP Response，找出 Body (跳過 Header)
    auto headerEnd = response.find("\r\n\r\n");
    if (headerEnd != std::string::npos) {
        std::string body = response.substr(headerEnd + 4);
        std::cout << "\n=== Online GPU Nodes ===\n";
        std::cout << body << "\n";
        std::cout << "========================\n";
    } else {
        std::cerr << "Invalid response from server.\n";
        return -1;
    }

    return 0;
}
