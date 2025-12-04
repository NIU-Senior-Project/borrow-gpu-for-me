#include "push.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

class PushTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Create a dummy script file
    std::ofstream outfile("test_script.sh");
    outfile << "#!/bin/bash\necho 'hello'" << std::endl;
    outfile.close();

    // Create a dummy non-script file
    std::ofstream txtfile("test_file.txt");
    txtfile << "hello" << std::endl;
    txtfile.close();
  }

  void TearDown() override {
    std::filesystem::remove("test_script.sh");
    std::filesystem::remove("test_file.txt");
  }
};

TEST_F(PushTest, FileDoesNotExist) {
  EXPECT_EQ(push("non_existent.sh", "container_id"), -1);
}

TEST_F(PushTest, InvalidExtension) {
  EXPECT_EQ(push("test_file.txt", "container_id"), -3);
}

TEST_F(PushTest, ValidScript) {
  // Note: This might fail if the container logic inside push() actually tries
  // to connect But based on the code, it returns 0 at the end. If it tries to
  // do network stuff, we might need to mock or expect failure. Looking at
  // push.cpp, it just returns 0 after checks.
  EXPECT_EQ(push("test_script.sh", "container_id"), 0);
}
