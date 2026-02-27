#include <iostream>
#include <string>
#include <sstream>

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

        // 清除輸入緩衝區，為接下來的 getline 準備
        std::cin.ignore(10000, '\n');

        // 3. 輸入要執行的 Script
        // 這裡為了簡單，先假設讀取單行指令 (例如 "nvidia-smi" 或 "python3 train.py")
        std::string script;
        std::cout << "Enter the command/script to execute on the remote GPU:\n> ";
        std::getline(std::cin, script);

        // 4. 送出任務
        send_job(manager_ip, script, target_ip, "nvidia/cuda:12.2.0-base-ubuntu22.04");

    } else {
        std::cerr << "Invalid choice. Exiting.\n";
        return 1;
    }

    return 0;
}
