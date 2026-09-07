#!/bin/bash

set -e

mkdir -p build

cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_OSX_ARCHITECTURES=arm64

cmake --build build --parallel

./build/creature_sim
