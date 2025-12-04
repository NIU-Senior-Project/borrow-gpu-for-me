#pragma once

#include <string>

int pull_container(std::string container);
int launch_container(std::string container);

std::string get_pull_command(std::string container);
std::string get_launch_command(std::string container);