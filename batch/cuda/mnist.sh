#!/usr/bin/env bash
set -e

apt update
apt install -y git curl ca-certificates

echo "=== Check GPU inside container ==="
nvidia-smi || { echo "GPU not available inside container"; exit 1; }

git clone https://github.com/Young-TW/MNIST.git

curl -LsSf https://astral.sh/uv/install.sh | sh
source "$HOME/.local/bin/env"

cd MNIST || exit 1

uv sync

echo "=== Python / Torch CUDA check ==="
uv run python -c "import torch; print('torch:', torch.__version__); print('cuda available:', torch.cuda.is_available()); print('device count:', torch.cuda.device_count())"

echo "=== Run main program ==="
uv run src/main.py