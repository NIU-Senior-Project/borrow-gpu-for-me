#include <sys/wait.h>

#include <cstdlib>
#include <string>

bool check_installed(std::string tool_name) {
    std::string command = "which " + tool_name + " > /dev/null 2>&1";
    int status = system(command.c_str());
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

bool server_tools_installed() {
    // Check for all required tools on the server
    return check_installed("docker") && check_installed("nvidia-smi") &&
           check_installed("ssh") && check_installed("scp") &&
           check_installed("nvcc");
}