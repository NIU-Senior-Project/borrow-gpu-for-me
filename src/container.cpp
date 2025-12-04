#include <cstdlib>
#include <string>
#include <sys/wait.h>

std::string get_pull_command(std::string container) {
  return "docker pull " + container + " > /dev/null 2>&1";
}

int pull_container(std::string container) {
  int status = system(get_pull_command(container).c_str());
  if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
    return -1; // Failed to pull container
  }

  return 0; // Success
}

std::string get_launch_command(std::string container) {
  std::string command = "docker run -d " + container + " > /dev/null 2>&1 \\ ";
  // command += "--device /dev/kfd --device /dev/dri "; // AMD GPU support
  command += "--gpus all "; // NVIDIA GPU support
  return command;
}

int launch_container(std::string container) {
  int status = system(get_launch_command(container).c_str());
  if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
    return -1; // Failed to launch container
  }

  return 0; // Success
}
