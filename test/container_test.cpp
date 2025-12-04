#include "container.h"

#include <gtest/gtest.h>

TEST(ContainerTest, GetPullCommand) {
    std::string container = "ubuntu:latest";
    std::string expected = "docker pull ubuntu:latest > /dev/null 2>&1";
    EXPECT_EQ(get_pull_command(container), expected);
}

TEST(ContainerTest, GetLaunchCommand) {
    std::string container = "nvidia/cuda:11.0-base";
    std::string command = get_launch_command(container);
    EXPECT_NE(command.find("docker run -d"), std::string::npos);
    EXPECT_NE(command.find(container), std::string::npos);
    EXPECT_NE(command.find("--gpus all"), std::string::npos);
}
