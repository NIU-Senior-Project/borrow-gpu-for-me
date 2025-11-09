#include <string>
#include <cstdlib>
#include <sys/wait.h>

int pull_container(std::string container) {
    int status = system(("docker pull " + container + " > /dev/null 2>&1").c_str());
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        return -1; // Failed to pull container
    }

    return 0; // Success
}
