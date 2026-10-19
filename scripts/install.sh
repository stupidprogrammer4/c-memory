#!/bin/sh
set -eu
project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
install_prefix=${1:-"$HOME/.local"}
build_dir=$project_dir/build/install
cmake -S "$project_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release
cmake --build "$build_dir" --parallel
ctest --test-dir "$build_dir" --output-on-failure
cmake --install "$build_dir" --prefix "$install_prefix"
printf 'C Memory installed under %s\n' "$install_prefix"
