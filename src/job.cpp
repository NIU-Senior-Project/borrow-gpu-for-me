#include "job.h"

#include <iostream>
#include <string>
#include <sstream>

// Socket 相關標頭檔 (給監聽 Server 用)
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <sys/wait.h>

// 簡單的 JSON 欄位提取
std::string extract_json_field(const std::string& body, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    auto keyPos = body.find(searchKey);
    if (keyPos == std::string::npos) return "";

    auto colonPos = body.find(':', keyPos);
    if (colonPos == std::string::npos) return "";

    auto firstQuote = body.find('"', colonPos + 1);
    if (firstQuote == std::string::npos) return "";

    auto secondQuote = body.find('"', firstQuote + 1);
    while (secondQuote != std::string::npos && body[secondQuote - 1] == '\\') {
        secondQuote = body.find('"', secondQuote + 1);
    }

    if (secondQuote == std::string::npos) return "";
    return body.substr(firstQuote + 1, secondQuote - firstQuote - 1);
}

// 核心：安全地執行 Docker 並擷取輸出
std::string run_docker_job(const std::string& container, const std::string& script) {
    int pipefd[2];
    // 建立管線，pipefd[0] 用來讀，pipefd[1] 用來寫
    if (pipe(pipefd) == -1) {
        return "[ERROR] Failed to create pipe for process communication.\n";
    }

    pid_t pid = fork();
    if (pid == -1) {
        return "[ERROR] Failed to fork process.\n";
    }

    if (pid == 0) {
        // -----------------------------------------
        // 子程序 (Child Process)：負責執行 Docker
        // -----------------------------------------
        close(pipefd[0]); // 子程序不需要讀，關閉讀取端

        // 將標準輸出 (stdout) 和標準錯誤 (stderr) 導向到 pipe
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        // 執行指令: docker run --rm --gpus all <container> /bin/bash -c "<script>"
        // docker + nvidia gpu

        execlp("docker",
               "docker", "run", "--rm", "--gpus", "all",
               container.c_str(),
               "/bin/bash", "-c", script.c_str(),
               nullptr);

        // podman + amd gpu
        /*
        execlp("podman",
               "podman", "run", "--rm",
               "--device", "/dev/kfd",
               "--device", "/dev/dri",
               container.c_str(),
               "/bin/bash", "-c", script.c_str(),
               nullptr);
        */
        // 如果 execlp 成功，它會替換掉當前程序，下面的程式碼永遠不會執行。
        // 如果執行到這裡，代表 Docker 啟動失敗（例如沒安裝 Docker）。
        std::cerr << "Failed to execute docker/podman command.\n";
        exit(1);
    } else {
        // -----------------------------------------
        // 父程序 (Parent Process)：負責讀取輸出
        // -----------------------------------------
        close(pipefd[1]); // 父程序不需要寫，關閉寫入端

        std::string output;
        char buffer[1024];
        ssize_t bytesRead;

        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            output += buffer;
        }
        close(pipefd[0]);

        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            int exitCode = WEXITSTATUS(status);
            if (exitCode != 0) {
                // 如果 Exit Code 不是 0，代表執行失敗，我們在輸出前面加上明顯的警告
                output = "[ERROR] Container exited with code " + std::to_string(exitCode) + "\n" +
                         "--- Container Output ---\n" + output;
            }
        } else if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            output = "[ERROR] Container killed by signal " + std::to_string(sig) + "\n" +
                     "--- Container Output ---\n" + output;
        }

        return output;
    }
}

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

        // 簡單讀取 Request (注意: 實務上可能需要根據 Content-Length 確保 Body 讀完)
        while ((bytesRead = recv(clientFd, buffer, sizeof(buffer), 0)) > 0) {
            request.append(buffer, bytesRead);
            if (request.find("\r\n\r\n") != std::string::npos) {
                // 如果是 POST，最好再多讀一下 Body
                break;
            }
        }

        auto headerEnd = request.find("\r\n\r\n");
        if (headerEnd != std::string::npos) {
            std::string body = request.substr(headerEnd + 4);

            // 1. 解析 JSON 取得參數
            std::string container = extract_json_field(body, "container");
            std::string script = extract_json_field(body, "script");

            // 簡單的反跳脫 (把 \\n 換回 \n，把 \\" 換回 ")
            // 實務上可以用 JSON 函式庫，這裡我們只處理最基本的
            size_t pos = 0;
            while ((pos = script.find("\\n", pos)) != std::string::npos) {
                script.replace(pos, 2, "\n");
                pos += 1;
            }
            pos = 0;
            while ((pos = script.find("\\\"", pos)) != std::string::npos) {
                script.replace(pos, 2, "\"");
                pos += 1;
            }

            std::cout << "\n[NEW JOB RECEIVED]\n";
            std::cout << "Container: " << container << "\n";
            std::cout << "Script:\n" << script << "\n";
            std::cout << "Executing...\n";

            // 2. 呼叫 Docker 執行並取得結果
            std::string execution_result = run_docker_job(container, script);

            std::cout << "--- Execution Output ---\n" << execution_result << "\n------------------------\n";
            std::cout << "[JOB FINISHED]\n";

            // 3. 將執行結果包裝成 HTTP 200 OK 回傳給 Manager
            std::ostringstream response;
            response << "HTTP/1.1 200 OK\r\n"
                     << "Content-Type: text/plain\r\n"
                     << "Content-Length: " << execution_result.size() << "\r\n"
                     << "Connection: close\r\n\r\n"
                     << execution_result;

            send(clientFd, response.str().data(), response.str().size(), 0);
        }
        close(clientFd);
    }
}
