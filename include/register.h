#pragma once

#include <string>

bool register_node(const std::string& manager_ip, int manager_port, const std::string& node_ip, const std::string& gpu_model);

int register_gpu(std::string gpu_model, std::string ip);

int view_online_gpus(std::string manager_ip);
