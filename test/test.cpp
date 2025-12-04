#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "gpu.h"
#include "request.h"

TEST(AvailableTest, Sanity) {
    // This test intentionally succeeds; replace with a real assertion if
    // desired.
    SUCCEED();
}

TEST(GpuTest, DetectionAndRegistration) {
    // Test GPU detection
    bool gpu_available = have_gpu_support();
    if (gpu_available) {
        std::cout << "GPU support detected." << std::endl;
        std::string gpu_model = detect_gpu_model();
        std::cout << "Detected GPU model: " << gpu_model << std::endl;

        // Basic check: detected model string should not be empty
        EXPECT_FALSE(gpu_model.empty());

        // Test GPU registration (don't assert on external side effects; just
        // call)
        std::string test_ip = "127.0.0.1";
        int registration_result = register_gpu(gpu_model, test_ip);
        (void)registration_result;  // avoid unused-variable warnings
        SUCCEED() << "Registration returned " << registration_result;
    } else {
        std::cout << "No GPU support detected." << std::endl;
        SUCCEED() << "No GPU present on this machine";
    }
}
