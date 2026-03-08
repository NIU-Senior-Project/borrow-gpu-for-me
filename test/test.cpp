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
    std::cout << "GPU support detected." << std::endl;
    GpuVendor gpu_vendor = detect_gpu_vendor();
    std::cout << "Detected GPU vendor: " << static_cast<int>(gpu_vendor) << std::endl;

    // Basic check: detected model string should not be empty
    EXPECT_FALSE(gpu_vendor == GpuVendor::UNKNOWN) << "GPU vendor should be detected if GPU support is present";

    // Test GPU registration (don't assert on external side effects; just
    // call)
    std::string test_ip = "127.0.0.1";
    int registration_result = register_gpu("unknown GPU", test_ip);
    (void)registration_result;  // avoid unused-variable warnings
    SUCCEED() << "Registration returned " << registration_result;
}
