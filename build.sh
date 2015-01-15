#!/usr/bin/env sh

set -e

rm -f build/challenges
clang -O0 -g challenges.cpp -o build/challenges && ./build/challenges
