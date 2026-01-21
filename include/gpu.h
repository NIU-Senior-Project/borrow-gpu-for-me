#pragma once

#include <string>

bool have_gpu_support();
std::string detect_gpu_model();

std::string get_check_nv_gpu_command();
std::string get_detect_nv_gpu_command();

std::string get_check_amd_gpu_command();
std::string get_detect_amd_gpu_command();
