#!/bin/env bash

apt update
apt install -y cmake git

git clone https://github.com/Young-TW/rocOdyssey.git

cd rocOdyssey || exit

cmake -B build -DODYSSEY_BACKEND=HIP -DCMAKE_BUILD_TYPE=Release

cmake --build build

./build/exec
