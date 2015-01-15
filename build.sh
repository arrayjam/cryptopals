#!/usr/bin/env sh

set -e

rm -r build/
mkdir -p build
clang -O0 -g challenges.cpp -o build/challenges && ./build/challenges
