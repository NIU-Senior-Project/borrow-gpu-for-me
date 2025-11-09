#include <filesystem>

int push(std::filesystem::path script, std::string container) {
    if (!std::filesystem::exists(script)) {
        return -1; // Script file does not exist
    }

    if (!std::filesystem::is_regular_file(script)) {
        return -2; // Not a regular file
    }

    if (script.extension() != ".sh") {
        return -3; // Invalid file extension
    }

    // get container ip and send script to container
    return 0;
}
