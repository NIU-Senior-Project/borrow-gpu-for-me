#include "job.h"

#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <fstream>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "json.h"

std::string generate_job_id() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return "job_" + std::to_string(ms);
}

// 非同步執行 Podman，並在結束時建立 .done 檔案
std::string run_docker_job_async(const std::string& container, const std::string& script) {
    std::string job_id = generate_job_id();
    std::string output_file = "/tmp/" + job_id + ".out";
    std::string done_file = "/tmp/" + job_id + ".done";

    pid_t pid = fork();
    if (pid == -1) return "ERROR_FORK_FAILED";

    if (pid == 0) {
        setsid(); 
        pid_t pid2 = fork();

        if (pid2 > 0) {
            exit(0); 
        } else if (pid2 == 0) {
            // 將輸出導向檔案
            int fd = open(output_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd != -1) {
                dup2(fd, STDOUT_FILENO);
                dup2(fd, STDERR_FILENO);
                close(fd);
            }

            std::cout << "[INFO] Job started.\n";

            // 第三次 fork 實際執行容器，讓父程序可以等待並寫入 .done 標記
            pid_t pid3 = fork();
            if (pid3 == 0) {
                // podman + ROCm
                /*
                execlp("podman",
                       "podman", "run", "--rm",
                       "--device", "/dev/kfd",
                       "--device", "/dev/dri",
                       container.c_str(),
                       "/bin/bash", "-c", script.c_str(),
                       nullptr);
                std::cerr << "[ERROR] Failed to execute podman command.\n";
                */
                // docker + CUDA
                // /*
                execlp("docker",
                       "docker", "run", "--rm",
                       "--gpus", "all",
                       container.c_str(),
                       "/bin/bash", "-c", script.c_str(),
                       nullptr);
                // */
                exit(1);
            } else {
                int status;
                waitpid(pid3, &status, 0);
                std::cout << "\n[INFO] Container exited.\n";

                // 容器執行完畢，建立標記檔案
                int fd_done = open(done_file.c_str(), O_WRONLY | O_CREAT, 0644);
                if (fd_done != -1) close(fd_done);
                exit(0);
            }
        }
    } else {
        waitpid(pid, nullptr, 0);
    }

    return job_id;
}

void start_job_listener(int port) {
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) return;

    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(serverFd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) return;
    if (listen(serverFd, 5) < 0) return;

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
            if (request.find("\r\n\r\n") != std::string::npos) break;
        }

        auto headerEnd = request.find("\r\n\r\n");
        if (headerEnd != std::string::npos) {
            std::string headerPart = request.substr(0, headerEnd);
            std::string body = request.substr(headerEnd + 4);

            std::istringstream stream(headerPart);
            std::string method, path, version;
            stream >> method >> path >> version;

            std::ostringstream response;

            // 路由 1：查詢狀態
            if (method == "GET" && path.find("/status?id=") == 0) {
                std::string jobId = path.substr(11);
                std::string out_file = "/tmp/" + jobId + ".out";
                std::string done_file = "/tmp/" + jobId + ".done";

                bool is_done = (access(done_file.c_str(), F_OK) == 0);

                // 讀取輸出檔案內容
                std::ifstream ifs(out_file);
                std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

                if (is_done) {
                    response << "HTTP/1.1 200 OK\r\nContent-Length: " << content.size() << "\r\nConnection: close\r\n\r\n" << content;
                } else {
                    // 還沒跑完，回傳 202 Accepted 與目前的輸出進度
                    response << "HTTP/1.1 202 Accepted\r\nContent-Length: " << content.size() << "\r\nConnection: close\r\n\r\n" << content;
                }
            } 
            // 路由 2：提交新任務
            else if (method == "POST") {
                std::string container = extract_json_field(body, "container");
                std::string script = extract_json_field(body, "script");

                if (!container.empty() && !script.empty()) {
                    size_t pos = 0;
                    while ((pos = script.find("\\n", pos)) != std::string::npos) { script.replace(pos, 2, "\n"); pos += 1; }
                    pos = 0;
                    while ((pos = script.find("\\\"", pos)) != std::string::npos) { script.replace(pos, 2, "\""); pos += 1; }

                    std::string job_id = run_docker_job_async(container, script);
                    std::cout << "\n[NEW JOB RECEIVED] Assigned Job ID: " << job_id << "\n";

                    std::ostringstream json_response;
                    json_response << "{\"job_id\": \"" << job_id << "\"}";
                    std::string resp_body = json_response.str();

                    response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resp_body.size() << "\r\nConnection: close\r\n\r\n" << resp_body;
                } else {
                    response << "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n";
                }
            } else {
                response << "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n";
            }

            send(clientFd, response.str().data(), response.str().size(), 0);
        }
        close(clientFd);
    }
}
