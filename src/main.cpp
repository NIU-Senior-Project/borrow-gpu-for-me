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

std::map<std::string, std::string> load_config(const std::string& filename) {
    std::map<std::string, std::string> config;
    std::ifstream file(filename);
    if (!file.is_open()) {
        return config;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        auto delimiter_pos = line.find('=');
        if (delimiter_pos != std::string::npos) {
            std::string key = line.substr(0, delimiter_pos);
            std::string value = line.substr(delimiter_pos + 1);
            config[key] = value;
        }
    }
    return config;
}

int main(int argc, char* argv[]) {
    // Read configuration at startup
    auto config = load_config(".config");
    std::string default_manager_ip = config.count("MANAGER_IP") ? config["MANAGER_IP"] : "";
    std::string default_node_ip = config.count("NODE_IP") ? config["NODE_IP"] : "";

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

        std::string manager_ip = default_manager_ip;
        std::string node_ip = default_node_ip;

        if (manager_ip.empty()) {
            std::cout << "Enter Node Manager IP (e.g., 127.0.0.1): ";
            std::cin >> manager_ip;
        } else {
            std::cout << "[INFO] Using Manager IP from config: " << manager_ip << "\n";
        }

        if (node_ip.empty()) {
            std::cout << "Enter this Node's IP (for others to connect): ";
            std::cin >> node_ip;
        } else {
            std::cout << "[INFO] Using Node IP from config: " << node_ip << "\n";
        }

        if (register_node(manager_ip, 8080, node_ip, "unknown_gpu_model")) {
            start_job_listener(8081);
        } else {
            std::cerr << "Registration failed. Shutting down.\n";
            return 1;
        }

    } else if (choice == 2) {
        std::string manager_ip = default_manager_ip;

        if (manager_ip.empty()) {
            std::cout << "Enter Node Manager IP (e.g., 127.0.0.1): ";
            std::cin >> manager_ip;
        } else {
            std::cout << "[INFO] Using Manager IP from config: " << manager_ip << "\n";
        }

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

        send_job(manager_ip, script, target_ip, "docker.io/rocm/dev-ubuntu-24.04:7.2");
    } else {
        std::cerr << "Invalid choice. Exiting.\n";
        return 1;
    }

    return 0;
}