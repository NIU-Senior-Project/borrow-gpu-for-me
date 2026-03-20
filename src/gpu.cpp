#include <sys/wait.h>

#include <cstdlib>
#include <fstream>
#include <string>
#include <cstdio>

#include "gpu.h"

GpuVendor detect_gpu_vendor() {
    if (access("/dev/nvidiactl", F_OK) == 0) {
        return GpuVendor::NVIDIA;
    } else if (access("/dev/kfd", F_OK) == 0) {
        return GpuVendor::AMD;
    }
    return GpuVendor::UNKNOWN;
}

static std::string run_cmd(const std::string& cmd) {
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";
    char buf[256];
    std::string result;
    while (fgets(buf, sizeof(buf), pipe))
        result += buf;
    pclose(pipe);
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
        result.pop_back();
    return result;
}

std::string detect_gpu_model() {
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

int detect_gpu_memory_mb() {
    GpuVendor vendor = detect_gpu_vendor();

    if (vendor == GpuVendor::NVIDIA) {
        std::string mem_str = run_cmd(
            "nvidia-smi --query-gpu=memory.total --format=csv,noheader 2>/dev/null");
        try {
            return std::stoi(mem_str);
        } catch (...) {
            return 0;
        }
    }
    if (vendor == GpuVendor::AMD) {
        std::string mem_str = run_cmd(
            "rocm-smi --showmeminfo vram 2>/dev/null"
            " | awk '/VRAM Total Memory/{print int($NF / 1048576); exit}'");
        try {
            return std::stoi(mem_str);
        } catch (...) {
            return 0;
        }
    }

    return 0;
}
