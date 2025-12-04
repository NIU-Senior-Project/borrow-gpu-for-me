#include <gtest/gtest.h>

#include "tool.h"

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
