#include <gtest/gtest.h>

#include "tool.h"

TEST(ToolTest, ServerToolsInstalled) {
    bool installed = server_tools_installed();
    if (installed) {
        SUCCEED() << "All required server tools are installed.";
    } else {
        FAIL() << "One or more required server tools are missing.";
    }
}
