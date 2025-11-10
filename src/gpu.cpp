#include <cstdlib>
#include <sys/wait.h>

#include <string>
#include <fstream>

bool have_gpu_support() {
    int status = system("nvidia-smi > /dev/null 2>&1");
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return true; // GPU is available
    }
    return false; // GPU is not available
}

std::string detect_gpu_model() {
    // Placeholder function to detect GPU model
    system("nvidia-smi --query-gpu=name --format=csv,noheader > gpu_model.txt");
    std::ifstream file("gpu_model.txt");
    std::string gpu_model;
    if (file.is_open()) {
        std::getline(file, gpu_model);
        file.close();
    }
    return gpu_model;
}
