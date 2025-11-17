cmake_minimum_required(VERSION 3.20.0)
project(borrow-gpu-for-me VERSION 0.0.0)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        52eb8108c5bdec04579160ae17225d66034bd723 # v1.17.0
)

include_directories(./include)

file(GLOB_RECURSE SRC_FILES ./src/*.cpp)

# Build a library from the implementation sources so tests can link to it.
# Exclude main.cpp from the library so the executable provides the program entry.
file(GLOB_RECURSE LIB_SRC_FILES ./src/*.cpp)
list(REMOVE_ITEM LIB_SRC_FILES ${CMAKE_SOURCE_DIR}/src/main.cpp)

add_library(${PROJECT_NAME}_lib STATIC ${LIB_SRC_FILES})
target_include_directories(${PROJECT_NAME}_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Build the main executable and link it to the library
add_executable(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)

install(TARGETS ${PROJECT_NAME})

FetchContent_MakeAvailable(googletest)

# 1. 查找測試原始碼檔案
file(GLOB_RECURSE TEST_SRC_FILES "./test/*.cpp")

# 2. 添加測試可執行目標
# 命名為 ${PROJECT_NAME}_test 是常見的慣例
add_executable(${PROJECT_NAME}_test ${TEST_SRC_FILES})

# 3. 連結到 Google Test 庫
# gtest_main 包含 main() 函式，用於運行所有測試
target_link_libraries(${PROJECT_NAME}_test
    PRIVATE
    gtest_main
    gtest
    # 把專案實作庫連結到測試，這會解決測試中找不到符號的連結錯誤
    ${PROJECT_NAME}_lib
)

# 4. 啟用 CTest 支援並註冊測試
enable_testing()
add_test(NAME ${PROJECT_NAME}_unit_tests COMMAND ${PROJECT_NAME}_test)