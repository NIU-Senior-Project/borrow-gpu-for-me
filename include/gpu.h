#pragma once

#include <string>

enum class GpuVendor { NVIDIA, AMD, UNKNOWN };

GpuVendor detect_gpu_vendor();

std::string get_check_nv_gpu_command();
std::string get_detect_nv_gpu_command();

std::string get_check_amd_gpu_command();
std::string get_detect_amd_gpu_command();
