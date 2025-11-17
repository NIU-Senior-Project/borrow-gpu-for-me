# Borrow-GPU-For-Me

這是一個讓使用者能夠借用他人閒置 GPU 資源的平臺。無論是進行深度學習訓練、渲染工作，還是其他需要高性能計算的任務，Borrow-GPU-For-Me 都能幫助你找到合適的 GPU 資源。

## 功能特色

- **GPU 資源共享**：使用者可以將自己的閒置 GPU 資源上傳至平臺，供其他使用者借用。
- **多樣化 GPU 型號**：支援各種不同型號的 GPU
- **安全保障**：採用容器化技術，確保借用過程中的數據安全和隱私保護。

## 使用說明

1. 下載並安裝 Borrow-GPU-For-Me 客戶端。

    ```bash
    git clone https://github.com/NIU-Senior-Project/borrow-gpu-for-me.git
    cd borrow-gpu-for-me
    cmake -B build
    cmake --build build --config RELEASE
    cmake --install build --config RELEASE
    ```

2. 將你的 GPU 資源上傳至平臺，或瀏覽並借用其他使用者的 GPU 資源。

    ```bash
    # 上傳 GPU 資源
    borrow-gpu-for-me --gpu
    ```

    ```bash
    # 瀏覽並借用 GPU 資源
    borrow-gpu-for-me --list
    ```

## 開發者指南

執行單元測試

執行前需要先編譯專案，請參考上方的使用說明。

```bash
ctest --test-dir build --output-on-failure
```
