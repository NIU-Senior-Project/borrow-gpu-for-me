#!/bin/env bash

git clone https://github.com/Young-TW/rocOdyssey.git

cd rocOdyssey || exit

apt install -y cmake

cmake -B build

cmake --build build

./build/rocOdyssey
