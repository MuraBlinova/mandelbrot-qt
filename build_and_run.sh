#!/bin/bash

mkdir -p build-release
cd build-release

cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=$(brew --prefix qt@5)

make -j$(sysctl -n hw.ncpu)

./src/App/demo-app