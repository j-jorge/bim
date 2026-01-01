#!/bin/bash

set -euo pipefail

: "${benchmark_repository:=https://github.com/j-jorge/benchmark/}"
: "${benchmark_version:=1.9.4.0j}"
package_revision=1
version="$benchmark_version"-"$package_revision"
build_type=release

! bim-install-package benchmark "$version" "$build_type" 2>/dev/null \
    || exit 0

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

# shellcheck source=SCRIPTDIR/set-package-vars.sh
. "$script_dir"/set-package-vars.sh benchmark "$build_type"

bim-git-clone-repository \
    "$benchmark_repository" "v$benchmark_version" "$source_dir"

bim-cmake-build \
        --build-dir "$build_dir" \
        --build-type "${build_type^}" \
        --install-dir "$install_dir" \
        --source-dir "$source_dir" \
        --cmake -DBENCHMARK_ENABLE_GTEST_TESTS=OFF \
        --cmake -DBENCHMARK_ENABLE_TESTING=OFF \
        --cmake -DBENCHMARK_USE_BUNDLED_GTEST=OFF

bim-package-and-install "$install_dir" benchmark "$version" "$build_type"
