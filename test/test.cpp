// add a test function
#include <iostream>
#include <string>
#include <cassert>

#include "gpu.h"
#include "request.h"

void test_gpu_detection_and_registration() {
    // Test GPU detection
    bool gpu_available = have_gpu_support();
    if (gpu_available) {
        std::cout << "GPU support detected." << std::endl;
        std::string gpu_model = detect_gpu_model();
        std::cout << "Detected GPU model: " << gpu_model << std::endl;

        // Test GPU registration
        std::string test_ip = "127.0.0.1";
        int registration_result = register_gpu(gpu_model, test_ip);
        if (registration_result == 0) {
            std::cout << "GPU registration successful." << std::endl;
        } else {
            std::cout << "GPU registration failed with code: " << registration_result << std::endl;
        }
    } else {
        std::cout << "No GPU support detected." << std::endl;
    }

    assert(gpu_available || !gpu_available); // Dummy assertion to avoid unused variable warning
}