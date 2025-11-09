#include <cstdlib>
#include <sys/wait.h>

bool have_gpu_support() {
    int status = system("nvidia-smi > /dev/null 2>&1");
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return true; // GPU is available
    }
    return false; // GPU is not available
}
