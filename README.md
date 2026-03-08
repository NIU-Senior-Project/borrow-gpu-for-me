# Borrow-GPU-For-Me

`Borrow-GPU-For-Me` 是一個以 C++ CLI 實作的 GPU 借用原型，目標是讓節點可以註冊可借出的 GPU，並讓借用端提交腳本到目標節點執行。

## 目前進度（2026-03-09）

目前主流程已可跑通到「提交任務並輪詢結果」：

- 已實作：GPU 支援檢查（NVIDIA/AMD）、GPU 型號偵測、節點註冊、線上節點查詢
- 已實作：借用端選擇目標節點、讀取本地腳本內容、提交工作到 Manager
- 已實作：提供端在註冊成功後啟動 Job Listener（`8081`），以 `podman` 非同步執行容器任務並提供狀態查詢
- 已驗證：`ctest --test-dir build --output-on-failure`，目前 `1/1` 測試通過

仍在原型階段：

- 任務排程、認證授權、錯誤復原、可觀測性等仍待完整化

## 系統需求

### 基本開發環境

- CMake >= 3.20
- C++17 編譯器（GCC/Clang）
- Git

### 提供端（Upload GPU Resource）

程式會檢查以下工具是否存在：

- `podman`
- `ssh`
- `scp`

GPU 偵測相關工具（依硬體擇一）：

- NVIDIA: `nvidia-smi`（以及 `nvcc` 供工具檢查）
- AMD: `rocm-smi`

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

## 測試

```bash
ctest --test-dir build --output-on-failure
```

目前測試涵蓋：

- 容器指令字串組裝（`podman pull` / `podman run`）
- 腳本檔案驗證（`push` 模組）
- GPU 偵測與註冊流程 smoke test

## 使用方式

### 啟動程式

```bash
./build/borrow-gpu-for-me
```

### 互動式選單

啟動後可選：

1. `Upload GPU Resource (Register & Listen)`
2. `Browse and Borrow GPU Resource (Send Job)`

### 模式 1: Upload GPU Resource（提供資源）

流程：

1. 檢查本機 GPU 支援
2. 檢查必要工具（`podman`/`ssh`/`scp`）
3. 偵測 GPU 型號
4. 輸入 Manager IP 與本機 Node IP
5. 註冊成功後啟動 Listener 監聽 `8081`

### 模式 2: Browse and Borrow GPU Resource（借用資源）

流程：

1. 輸入 Manager IP
2. 取得並顯示線上節點
3. 輸入目標 Node IP
4. 輸入腳本檔案路徑
5. 提交任務，持續輪詢並輸出遠端執行結果

腳本可參考：

- `batch/cuda.sh`
- `batch/hip.sh`
- `batch/cuda/mnist.sh`
- `batch/hip/mnist.sh`

## Node Manager API 假設

目前客戶端假設 Manager 監聽 `8080`，支援：

- `POST /register`：註冊節點（body: `ip`, `gpu_model`）
- `GET /nodes`：查詢線上節點
- `POST /submit_job`：提交任務（body: `target_node`, `container`, `script`）
- `GET /job_status?id=<job_id>&node=<node_ip>`：查詢任務狀態與輸出

同時假設 Manager 會轉送到節點端 Listener（`8081`）介面：

- `POST /`：提交容器與腳本
- `GET /status?id=<job_id>`：查詢任務輸出與完成狀態

若你的後端 API 格式不同，請同步調整 `src/register.cpp`、`src/request.cpp` 與 `src/job.cpp`。

## 專案結構

```text
include/   # 標頭檔（GPU、註冊、請求、容器、工具）
src/       # 核心實作
test/      # GoogleTest 測試
batch/     # 範例工作腳本
proto/     # proto 定義（預留）
```

## 常見問題

### `No GPU support detected`

- 確認驅動與工具可正常使用（`nvidia-smi` 或 `rocm-smi`）
- 確認執行環境可存取 GPU

### `Required server tools are not installed`

- 安裝並確認以下指令可在 PATH 中找到：`podman`、`ssh`、`scp`

### 註冊或提交工作失敗

- 確認 Manager IP/Port 正確（預設 `8080`）
- 確認網路可通，且 Manager 實作上述 API

## 後續方向

- 補齊 Manager 與 Node 端協議文件（含錯誤碼）
- 強化容器映像與腳本安全性檢查
- 加入非互動式 CLI 參數（CI/CD 友善）
- 擴充任務排程、資源配額與權限控管
