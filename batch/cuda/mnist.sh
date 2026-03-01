#!/bin/env bash

apt update
apt install -y git curl

git clone https://github.com/Young-TW/MNIST.git

# install uv
curl -LsSf https://astral.sh/uv/install.sh | sh
source $HOME/.local/bin/env

cd MNIST || exit

uv venv
uv pip install torch torchvision torchaudio

uv run src/main.py
