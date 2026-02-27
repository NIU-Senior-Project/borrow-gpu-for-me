# Borrow-GPU-For-Me

Borrow-GPU-For-Me 是一個讓使用者分享與借用閒置 GPU 資源的平臺雛形。專案目前提供 C++ CLI 客戶端，能協助你：

- 偵測本機是否具備 GPU 環境
- 讀取 GPU 型號資訊
- 向 Node Manager 註冊可借用節點
- 查詢線上節點清單

## 功能特色

- **GPU 資源共享**：提供節點註冊與節點列表查詢流程。
- **多樣化 GPU 型號**：支援 NVIDIA / AMD 偵測流程（依賴系統工具）。
- **容器執行基礎能力**：包含容器拉取、啟動與腳本檔案檢查等基礎模組。

## 專案狀態

目前版本為「客戶端原型」，以互動式選單操作為主，並假設後端已有 Node Manager 服務。

- 已實作：GPU 偵測、節點註冊 HTTP 請求、節點列表查詢 HTTP 請求、基礎容器/推送模組
- 未完整串接：工作派送完整流程、容器內腳本上傳與執行細節

## 系統需求

### 必要工具

- CMake >= 3.20
- C++17 編譯器（GCC / Clang）
- Git

### 依情境需求

若你要使用「上傳 GPU 資源（註冊節點）」模式，程式會檢查以下工具：

- `docker`
- `ssh`
- `scp`

若你要偵測 NVIDIA GPU，通常需有：

- `nvidia-smi`

若你要偵測 AMD GPU，通常需有：

- `rocm-smi`

## 安裝與編譯

```bash
git clone https://github.com/NIU-Senior-Project/borrow-gpu-for-me.git
cd borrow-gpu-for-me
cmake -B build
cmake --build build --config Release
```

可選安裝：

```bash
cmake --install build --config Release
```

## 使用方式

### 啟動程式

若在 build 目錄直接執行：

```bash
./build/borrow-gpu-for-me
```

若已安裝到系統路徑，則可直接執行：

```bash
borrow-gpu-for-me
```

### 互動式選單

程式啟動後會顯示：

1. `Upload GPU Resource`
2. `Browse and Borrow GPU Resource`

#### 1) Upload GPU Resource（提供資源）

流程：

1. 檢查本機 GPU 支援
2. 檢查伺服端必要工具（docker/ssh/scp）
3. 偵測 GPU 型號
4. 輸入 Node Manager IP 與本機 Node IP
5. 發送註冊請求至 Node Manager

#### 2) Browse and Borrow GPU Resource（查詢資源）

流程：

1. 輸入 Node Manager IP
2. 對 Node Manager 發送查詢請求
3. 顯示回傳的線上節點資訊

## Node Manager API 假設

目前客戶端預設 Node Manager 監聽 `8080` 並支援以下端點：

- `POST /register`：註冊節點（JSON body 會含 `ip`, `gpu_model`）
- `GET /nodes`：取得線上節點列表
- `POST /submit_job`：提交工作（目前程式有函式，但主流程尚未完整串接）

若你的後端 API 格式不同，請同步調整 `src/register.cpp` 與 `src/request.cpp`。

## 開發者指南

### 執行單元測試

先完成編譯後執行：

```bash
ctest --test-dir build --output-on-failure
```

### 測試內容概覽

- 容器命令字串組裝
- 腳本檔案檢查（push 模組）
- GPU 偵測/註冊函式基本流程
- 伺服端工具與 NVIDIA 工具存在檢查

## 專案結構

```text
include/   # 標頭檔（GPU、註冊、請求、容器、工具等介面）
src/       # 核心實作
test/      # GoogleTest 測試
proto/     # 相關 proto 定義（預留/擴充）
```

## 常見問題

### 1) 程式顯示 `No GPU support detected`

- 請確認驅動與工具可正常使用（例如 `nvidia-smi` 或 `rocm-smi`）
- 確認執行環境有可存取 GPU（例如非受限容器）

### 2) 註冊失敗或無法連線 Manager

- 確認 Node Manager IP 與埠號是否正確（預設 8080）
- 確認防火牆與網路路由可連線
- 確認 Node Manager 有實作對應 API

### 3) `Required server tools are not installed`

- 安裝並確認以下指令可在 shell 中找到：`docker`, `ssh`, `scp`

## 後續可擴充方向

- 完整化工作提交與排程回報流程
- 增加 CLI 參數模式（取代純互動式輸入）
- 強化 API 錯誤處理與結構化回應解析
- 擴充安全性與權限控管機制
