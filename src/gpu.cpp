#include <sys/wait.h>

#include <cstdlib>
#include <fstream>
#include <string>

std::string get_check_nv_gpu_command() { return "nvidia-smi > /dev/null 2>&1"; }
std::string get_check_amd_gpu_command() { return "rocm-smi > /dev/null 2>&1"; }


bool have_gpu_support() {
    int status = system(get_check_nv_gpu_command().c_str());
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return true;  // GPU is available
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) == 127) {
        status = system(get_check_amd_gpu_command().c_str());
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return true;  // AMD GPU is available
        }
    }

    return false;  // GPU is not available
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
