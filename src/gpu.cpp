#include <sys/wait.h>

#include <cstdlib>
#include <fstream>
#include <string>
#include <cstdio>

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
    return "rocm-smi --showproductname | awk -F 'Card Series:[[:space:]]*' '/Card Series/{print $2; exit}' > gpu_model.txt";
}

std::string detect_gpu_model() {
    auto run_cmd = [](const std::string& cmd) -> std::string {
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) return "";
        char buf[256];
        std::string result;
        while (fgets(buf, sizeof(buf), pipe)) {
            result += buf;
        }
        pclose(pipe);
        // trim trailing newline
        while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
            result.pop_back();
        return result;
    };

    GpuVendor vendor = detect_gpu_vendor();

    if (vendor == GpuVendor::NVIDIA) {
        std::string model = run_cmd(
            "nvidia-smi --query-gpu=name --format=csv,noheader 2>/dev/null");
        if (!model.empty()) return model;
    }
    if (vendor == GpuVendor::AMD) {
        std::string model = run_cmd(
            "rocm-smi --showproductname 2>/dev/null"
            " | awk -F 'Card Series:[[:space:]]*' '/Card Series/{print $2; exit}'");
        if (!model.empty()) return model;
    }

    return "Unknown GPU";
}
