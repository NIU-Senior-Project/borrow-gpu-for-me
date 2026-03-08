#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <map>

#include "gpu.h"
#include "tool.h"
#include "register.h"
#include "request.h"
#include "job.h"
#include "config.h"

int main(int argc, char* argv[]) {
    auto config = load_config(".config");

    std::cout << "Select Mode:\n";
    std::cout << "1. Upload GPU Resource (Register & Listen)\n";
    std::cout << "2. Browse and Borrow GPU Resource (Send Job)\n";
    std::cout << "> ";

    int choice;
    if (!(std::cin >> choice)) return 1;

    if (choice == 1) {
        GpuVendor gpu_vendor = detect_gpu_vendor();
        if (gpu_vendor == GpuVendor::UNKNOWN) {
            std::cerr << "No GPU support detected. Exiting.\n";
            return -1;
        } else if (gpu_vendor == GpuVendor::NVIDIA) {
            std::cout << "NVIDIA GPU detected.\n";
        } else if (gpu_vendor == GpuVendor::AMD) {
            std::cout << "AMD GPU detected.\n";
        }

        if (!server_tools_installed()) {
            std::cerr << "Required server tools are not installed. Exiting.\n";
            return -1;
        }

        std::string manager_ip = get_value(config, "MANAGER_IP");
        std::string node_ip = get_value(config, "NODE_IP");

        if (register_node(manager_ip, 8080, node_ip, "unknown_gpu_model")) {
            start_job_listener(8081);
        } else {
            std::cerr << "Registration failed. Shutting down.\n";
            return 1;
        }

    } else if (choice == 2) {
        std::string manager_ip = get_value(config, "MANAGER_IP");

        view_online_gpus(manager_ip);

        std::string target_ip;
        std::cout << "\nEnter target Node IP from the list above: ";
        std::cin >> target_ip;

        std::cin.ignore(10000, '\n');

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

        std::string container_image = get_config_value(config, "CONTAINER_IMAGE", "docker.io/rocm/dev-ubuntu-24.04:7.2");

        send_job(manager_ip, script, target_ip, container_image);
    } else {
        std::cerr << "Invalid choice. Exiting.\n";
        return 1;
    }

    return 0;
}