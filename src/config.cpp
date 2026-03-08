#include "config.h"

#include <string>
#include <map>
#include <fstream>
#include <iostream>

std::map<std::string, std::string> load_config(const std::string& filename) {
    std::map<std::string, std::string> config;
    std::ifstream file(filename);
    if (!file.is_open()) {
        return config;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        auto delimiter_pos = line.find('=');
        if (delimiter_pos != std::string::npos) {
            std::string key = line.substr(0, delimiter_pos);
            std::string value = line.substr(delimiter_pos + 1);
            config[key] = value;
        }
    }
    return config;
}

std::string get_config_value(const std::map<std::string, std::string>& config, const std::string& key, const std::string& default_value = "") {
    auto it = config.find(key);
    if (it != config.end()) {
        return it->second;
    }
    return default_value;
}

std::string get_value(const std::map<std::string, std::string>& config, const std::string& key) {
    if (config.count(key)) {
        return config.at(key);
    } else {
        std::cout << "Enter value for " << key << ": ";
        std::string value;
        std::cin >> value;
        return value;
    }
}
