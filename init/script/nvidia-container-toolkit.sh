#!/bin/bash

# 只要有任何指令回傳非 0 的錯誤碼，腳本就會立即停止執行，避免錯誤連鎖擴大
set -e

echo "[INFO] 正在加入 NVIDIA Container Toolkit GPG 金鑰..."
# 加上 --yes 參數，避免重複執行腳本時 GPG 詢問是否覆寫檔案而卡住
curl -fsSL https://nvidia.github.io/libnvidia-container/gpgkey | sudo gpg --dearmor --yes -o /usr/share/keyrings/nvidia-container-toolkit-keyring.gpg

echo "[INFO] 正在設定 APT 軟體庫來源..."
# 將 tee 的輸出導向 /dev/null，保持終端機畫面整潔
curl -s -L https://nvidia.github.io/libnvidia-container/stable/deb/nvidia-container-toolkit.list | \
    sed 's#deb https://#deb [signed-by=/usr/share/keyrings/nvidia-container-toolkit-keyring.gpg] https://#g' | \
    sudo tee /etc/apt/sources.list.d/nvidia-container-toolkit.list > /dev/null

echo "[INFO] 正在更新 APT 索引並安裝 nvidia-container-toolkit..."
sudo apt-get update
sudo apt-get install -y nvidia-container-toolkit

echo "[INFO] 正在產生 Podman CDI 設定檔..."
# 確保 /etc/cdi 目錄存在，避免 nvidia-ctk 找不到路徑報錯
sudo mkdir -p /etc/cdi
sudo nvidia-ctk cdi generate --output=/etc/cdi/nvidia.yaml

echo "[SUCCESS] 安裝與設定完成！"
echo "請執行 'nvidia-ctk cdi list' 來確認系統是否有抓到 nvidia.com/gpu 等裝置。"
