#pragma once

#include <string>
#include <map>

std::map<std::string, std::string> load_config(const std::string& filename);

std::string get_config_value(const std::map<std::string, std::string>& config, const std::string& key, const std::string& default_value);

std::string get_value(const std::map<std::string, std::string>& config, const std::string& key);
