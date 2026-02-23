#include <functional>
#include <iostream>
#include <sstream>
#include <string>

namespace {

std::string build_node_id(const std::string& gpu_model,
                          const std::string& ip) {
    const auto hash_value = std::hash<std::string>{}(gpu_model + "@" + ip);
    std::ostringstream os;
    os << "node-" << std::hex << hash_value;
    return os.str();
}

}  // namespace

int register_gpu(std::string gpu_model, std::string ip) {
    if (gpu_model.empty() || ip.empty()) {
        std::cerr << "register_gpu failed: gpu_model/ip must not be empty"
                  << std::endl;
        return -1;
    }

    const std::string node_id = build_node_id(gpu_model, ip);
    const int gpu_memory_mb = 0;

    std::ostringstream payload;
    payload << "RegisterNodeRequest {\n"
            << "  node_id: \"" << node_id << "\"\n"
            << "  node_address: \"" << ip << "\"\n"
            << "  gpu_model: \"" << gpu_model << "\"\n"
            << "  gpu_memory_mb: " << gpu_memory_mb << "\n"
            << "}";

    std::cout << payload.str() << std::endl;
    return 0;
}
