#pragma once

#include <string>

enum class GpuVendor { NVIDIA, AMD, UNKNOWN };

GpuVendor detect_gpu_vendor();
std::string detect_gpu_model();
