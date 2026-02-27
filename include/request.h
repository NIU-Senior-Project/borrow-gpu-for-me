#pragma once

#include <string>

int register_gpu(std::string gpu_model, std::string ip);

int view_online_gpus(std::string manager_ip);

std::string escape_json(const std::string& s);

int send_job(std::string manager_ip, std::string job_script, std::string node, std::string container);
