#!/usr/bin/env sh

set -e

rm -f formats
clang -O0 -g formats.cpp -o formats && ./formats "foo"
