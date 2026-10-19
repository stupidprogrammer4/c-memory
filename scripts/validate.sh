#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
validation_dir=$project_dir/build/validation
gcc_command=${CMEMORY_GCC:-gcc}
clang_command=${1:-clang}

command -v "$gcc_command" >/dev/null
command -v "$clang_command" >/dev/null
"$gcc_command" --version
"$clang_command" --version

run_configuration() {
    configuration_name=$1
    compiler=$2
    build_type=$3
    shared=$4
    sanitizers=$5
    configuration_dir=$validation_dir/$configuration_name
    cmake -S "$project_dir" -B "$configuration_dir" \
        -DCMAKE_C_COMPILER="$compiler" -DCMAKE_BUILD_TYPE="$build_type" \
        -DBUILD_SHARED_LIBS="$shared" -DCMEMORY_ENABLE_SANITIZERS="$sanitizers" \
        -DCMEMORY_BUILD_BENCHMARKS=ON -DCMEMORY_WARNINGS_AS_ERRORS=ON
    cmake --build "$configuration_dir" --parallel
    ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 \
        ctest --test-dir "$configuration_dir" --output-on-failure
}

for compiler_name in gcc clang; do
    if [ "$compiler_name" = gcc ]; then compiler=$gcc_command; else compiler=$clang_command; fi
    run_configuration "$compiler_name-debug" "$compiler" Debug OFF OFF
    run_configuration "$compiler_name-release" "$compiler" Release OFF OFF
    run_configuration "$compiler_name-shared" "$compiler" Release ON OFF
    run_configuration "$compiler_name-sanitized" "$compiler" Debug OFF ON
    "$validation_dir/$compiler_name-release/allocation_benchmark" \
        > "$validation_dir/$compiler_name-benchmark.csv"
done

cmake -S "$project_dir" -B "$validation_dir/gcc-analyzer" \
    -DCMAKE_C_COMPILER="$gcc_command" -DBUILD_TESTING=OFF -DCMEMORY_BUILD_EXAMPLES=OFF \
    -DCMEMORY_BUILD_BENCHMARKS=OFF '-DCMAKE_C_FLAGS=-fanalyzer -Werror'
cmake --build "$validation_dir/gcc-analyzer" --parallel
for source in "$project_dir"/src/core/*.c "$project_dir"/src/allocators/*.c \
    "$project_dir"/src/diagnostics/*.c; do
    "$clang_command" --analyze -std=c17 -Wall -Wextra -Werror \
        -I"$project_dir/include" -Xanalyzer -analyzer-output=text "$source"
done
printf 'Local compiler matrix, sanitizers, analyzers and benchmark smoke checks passed.\n'
