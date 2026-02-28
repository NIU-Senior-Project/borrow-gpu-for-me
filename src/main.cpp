#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#include "gpu.h"
#include "tool.h"
#include "register.h"
#include "request.h"
#include "job.h"

int main(int argc, char* argv[]) {
    std::cout << "Select Mode:\n";
    std::cout << "1. Upload GPU Resource (Register & Listen)\n";
    std::cout << "2. Browse and Borrow GPU Resource (Send Job)\n";
    std::cout << "> ";

    int choice;
    if (!(std::cin >> choice)) return 1;

    if (choice == 1) {
        if (!have_gpu_support()) {
            std::cerr << "No GPU support detected. Exiting.\n";
            return -1;
        }

        if (!server_tools_installed()) {
            std::cerr << "Required server tools are not installed. Exiting.\n";
            return -1;
        }

        std::string gpu_model = detect_gpu_model();
        std::cout << "Detected GPU Model: " << gpu_model << "\n";

        // 取得 Manager 與本機 Node 的 IP
        std::string manager_ip, node_ip;
        std::cout << "Enter Node Manager IP (e.g., 127.0.0.1): ";
        std::cin >> manager_ip;
        std::cout << "Enter this Node's IP (for others to connect): ";
        std::cin >> node_ip;

        // 註冊本機資源
        if (register_node(manager_ip, 8080, node_ip, gpu_model)) {
            // 註冊成功後，啟動 Server 監聽 8081 Port 準備接任務！
            start_job_listener(8081); 
        } else {
            std::cerr << "Registration failed. Shutting down.\n";
            return 1;
        }

    } else if (choice == 2) {
        std::string manager_ip;
        std::cout << "Enter Node Manager IP (e.g., 127.0.0.1): ";
        std::cin >> manager_ip;

        // 1. 查看線上節點
        view_online_gpus(manager_ip);

        // 2. 讓使用者選擇目標 IP
        std::string target_ip;
        std::cout << "\nEnter target Node IP from the list above: ";
        std::cin >> target_ip;

        // Clear input buffer for getline
        std::cin.ignore(10000, '\n');

        // 3. 讀取腳本檔案
        std::string script_path;
        std::cout << "Enter the path to the job script file:\n> ";
        std::getline(std::cin, script_path);

        std::ifstream script_file(script_path);
        if (!script_file.is_open()) {
            std::cerr << "[ERROR] Failed to open script file: " << script_path << "\n";
            return 1;
        }

        std::stringstream buffer;
        buffer << script_file.rdbuf();
        std::string script = buffer.str();

        // 4. 送出任務
        send_job(manager_ip, script, target_ip, "docker.io/rocm/dev-ubuntu-24.04:7.2");
    }  else {
        std::cerr << "Invalid choice. Exiting.\n";
        return 1;
    }

    return 0;
}
