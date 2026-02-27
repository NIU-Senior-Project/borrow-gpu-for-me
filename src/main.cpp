#include <iostream>
#include <string>
#include <sstream>

#include "gpu.h"
#include "tool.h"
#include "register.h"

int main(int argc, char* argv[]) {
    std::cout << "Select Mode:\n";
    std::cout << "1. Upload GPU Resource\n";
    std::cout << "2. Browse and Borrow GPU Resource\n";
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

        // Upload GPU Resource (Register)
        register_node(manager_ip, 8080, node_ip, gpu_model);

    } else if (choice == 2) {
        std::string manager_ip;
        std::cout << "Enter Node Manager IP (e.g., 127.0.0.1): ";
        std::cin >> manager_ip;

        view_online_gpus(manager_ip);
    } else {
        std::cerr << "Invalid choice. Exiting.\n";
        return 1;
    }

    return 0;
}