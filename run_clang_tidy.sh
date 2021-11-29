#!/usr/bin/env bash

clang-tidy \
    -extra-arg="-std=c++17" \
    -extra-arg="-I." \
    -extra-arg="-isystem/usr/include/freetype2" \
    -extra-arg="-isystem $(clang -print-resource-dir)/include" \
    -fix \
    $(git ls-files "*.cpp" "*.hpp")
