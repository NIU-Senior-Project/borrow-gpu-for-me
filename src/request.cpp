#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <thread>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "json.h"

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

// 簡單的 JSON 字串跳脫函式，避免 script 裡的換行或引號把 JSON 弄壞
std::string escape_json(const std::string& s) {
    std::ostringstream o;
    for (char c : s) {
        if (c == '"') o << "\\\"";
        else if (c == '\\') o << "\\\\";
        else if (c == '\b') o << "\\b";
        else if (c == '\f') o << "\\f";
        else if (c == '\n') o << "\\n";
        else if (c == '\r') o << "\\r";
        else if (c == '\t') o << "\\t";
        else o << c;
    }
    return o.str();
}

// Helper function to send simple HTTP GET
std::string http_get(const std::string& ip, int port, const std::string& path) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return "";

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    if (connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        close(sock);
        return "";
    }

    std::ostringstream request;
    request << "GET " << path << " HTTP/1.1\r\n"
            << "Host: " << ip << ":" << port << "\r\n"
            << "Connection: close\r\n\r\n";

    std::string req_str = request.str();
    send(sock, req_str.data(), req_str.size(), 0);

    std::string response;
    char buffer[4096];
    ssize_t bytesRead;
    while ((bytesRead = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesRead] = '\0';
        response.append(buffer);
    }
    close(sock);
    return response;
}

int send_job(std::string manager_ip, std::string job_script, std::string node, std::string container = "docker.io/rocm/dev-ubuntu-24.04:7.2") {
    // ... (Keep existing socket creation and POST request to /submit_job)
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    sockaddr_in manager_addr{};
    manager_addr.sin_family = AF_INET;
    manager_addr.sin_port = htons(8080);
    inet_pton(AF_INET, manager_ip.c_str(), &manager_addr.sin_addr);
    connect(sock, reinterpret_cast<sockaddr*>(&manager_addr), sizeof(manager_addr));

    std::ostringstream body;
    body << "{\n"
         << "  \"target_node\": \"" << node << "\",\n"
         << "  \"container\": \"" << container << "\",\n"
         << "  \"script\": \"" << escape_json(job_script) << "\"\n"
         << "}";
    std::string body_str = body.str();

    std::ostringstream request;
    request << "POST /submit_job HTTP/1.1\r\n"
            << "Host: " << manager_ip << ":8080\r\n"
            << "Content-Type: application/json\r\n"
            << "Content-Length: " << body_str.size() << "\r\n"
            << "Connection: close\r\n\r\n"
            << body_str;

    std::string req_str = request.str();
    send(sock, req_str.data(), req_str.size(), 0);

    std::string response;
    char buffer[4096];
    ssize_t bytesRead;
    while ((bytesRead = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesRead] = '\0';
        response.append(buffer);
    }
    close(sock);

    auto headerEnd = response.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        std::cerr << "[ERROR] Invalid response from Manager.\n";
        return -1;
    }

    std::string resp_body = response.substr(headerEnd + 4);
    
    // We expect the Manager to parse the Node's response and return the job_id
    // For simplicity, assuming extract_json_field is available here or implemented similarly
    std::string job_id = extract_json_field(resp_body, "job_id"); 
    
    if (job_id.empty()) {
        std::cerr << "[ERROR] Failed to get Job ID. Response:\n" << resp_body << "\n";
        return -1;
    }

    std::cout << "[SUCCESS] Job submitted! Job ID: " << job_id << "\n";
    std::cout << "Waiting for execution to complete...\n";

    // Polling Loop
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // Ask Manager for job status (Manager routes this to Node to read /tmp/job_id.out)
        std::string status_path = "/job_status?id=" + job_id + "&node=" + node;
        std::string status_resp = http_get(manager_ip, 8080, status_path);

        auto statusHeaderEnd = status_resp.find("\r\n\r\n");
        if (statusHeaderEnd != std::string::npos) {
            std::string status_body = status_resp.substr(statusHeaderEnd + 4);
            
            // Assume Manager returns HTTP 202 if still running, HTTP 200 when done
            if (status_resp.find("HTTP/1.1 202") != std::string::npos) {
                std::cout << "." << std::flush;
            } else if (status_resp.find("HTTP/1.1 200") != std::string::npos) {
                std::cout << "\n\n=== Remote Execution Result ===\n";
                std::cout << status_body << "\n";
                std::cout << "===============================\n";
                break;
            } else {
                std::cerr << "\n[ERROR] Unexpected status response.\n" << status_body << "\n";
                break;
            }
        }
    }

    return 0;
}
