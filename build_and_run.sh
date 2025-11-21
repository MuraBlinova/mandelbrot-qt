#!/bin/bash

mkdir -p build-release
cd build-release

cmake ..

make -j$(sysctl -n hw.ncpu)

./src/App/demo-app