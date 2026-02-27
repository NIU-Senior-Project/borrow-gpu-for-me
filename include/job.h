#pragma once

#include <string>

#include <sys/wait.h>

std::string extract_json_field(const std::string& body, const std::string& key);
std::string run_docker_job(const std::string& container, const std::string& script);

void start_job_listener(int port);
