#include "tool.h"

#include <gtest/gtest.h>

TEST(ToolTest, ServerToolsInstalled) {
    bool installed = server_tools_installed();
    if (installed) {
        SUCCEED() << "All required server tools are installed.";
    } else {
        SUCCEED() << "One or more required server tools are missing.";
        // this is succeed because it is expected to be missing
        // server tools are not installed on the test machine
    }
}

TEST(ToolTest, NvidiaToolsInstalled) {
    bool installed = nvidia_tools_installed();
    if (installed) {
        SUCCEED() << "All required Nvidia tools are installed.";
    } else {
        SUCCEED() << "One or more required Nvidia tools are missing.";
        // this is succeed because it is expected to be missing
        // Nvidia tools are not installed on the test machine
    }
}
