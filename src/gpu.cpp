#include <sys/wait.h>

#include <cstdlib>
#include <fstream>
#include <string>

#include "gpu.h"

std::string get_check_nv_gpu_command() { return "nvidia-smi > /dev/null 2>&1"; }
std::string get_check_amd_gpu_command() { return "rocm-smi > /dev/null 2>&1"; }

GpuVendor detect_gpu_vendor() {
    if (access("/dev/nvidiactl", F_OK) == 0) {
        return GpuVendor::NVIDIA;
    } else if (access("/dev/kfd", F_OK) == 0) {
        return GpuVendor::AMD;
    }
    return GpuVendor::UNKNOWN;
}

std::string get_detect_nv_gpu_command() {
    return "nvidia-smi --query-gpu=name --format=csv,noheader > gpu_model.txt";
}

std::string get_detect_amd_gpu_command() {
    return "rocm-smi --showproductname | grep 'GPU' | awk -F ': ' '{print $2}' > gpu_model.txt";
}

std::string detect_gpu_model() {
    // Placeholder function to detect GPU model
    int status = system(get_detect_nv_gpu_command().c_str());
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        std::ifstream file("gpu_model.txt");
        std::string gpu_model;
        if (file.is_open()) {
            std::getline(file, gpu_model);
            file.close();
            return gpu_model;
        }
    }

    system(get_detect_amd_gpu_command().c_str());
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        std::ifstream file("gpu_model.txt");
        std::string gpu_model;
        if (file.is_open()) {
            std::getline(file, gpu_model);
            file.close();
            return gpu_model;
        }
    }

    std::ifstream file("gpu_model.txt");
    std::string gpu_model;
    if (file.is_open()) {
        std::getline(file, gpu_model);
        file.close();
    }
    return gpu_model;
}
