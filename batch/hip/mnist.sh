#!/bin/env bash

apt update
apt install -y git curl

git clone https://github.com/Young-TW/MNIST.git

# install uv
curl -LsSf https://astral.sh/uv/install.sh | sh
source $HOME/.local/bin/env

cd MNIST || exit

uv pip install torch torchvision torchaudio \
    --index-url https://download.pytorch.org/whl/rocm6.4

uv run src/main.py
