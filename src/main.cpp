#include <iostream>

#include "gpu.h"

int main(int argc, char* argv[]) {
    std::cout << "Select Mode:" << std::endl;
    std::cout << "1. Upload GPU Resource" << std::endl;
    std::cout << "2. Browse and Borrow GPU Resource" << std::endl;
    int choice;
    std::cin >> choice;
    if (choice == 1) {
        if (!have_gpu_support()) {
            std::cerr << "No GPU support detected. Exiting." << std::endl;
            return -1;
        }

        std::string gpu_model = detect_gpu_model();
        std::cout << "Detected GPU Model: " << gpu_model << std::endl;
        // Upload GPU Resource
    } else if (choice == 2) {
        // Browse and Borrow GPU Resource
    }

    return 0;
}
