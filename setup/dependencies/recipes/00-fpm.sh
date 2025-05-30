#!/bin/bash

set -euo pipefail

: "${fpm_repository:=https://github.com/j-jorge/fpm.git/}"
: "${fpm_version:=1.1.1}"
package_revision=2
package_version="$fpm_version"-"$package_revision"
build_type=release

! bim-install-package fpm "$package_version" "$build_type" 2>/dev/null \
    || exit 0

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

# shellcheck source=SCRIPTDIR/set-package-vars.sh
. "$script_dir"/set-package-vars.sh fpm "$build_type"

bim-git-clone-repository "$fpm_repository" "v$fpm_version" "$source_dir"

bim-cmake-build \
    --build-dir "$build_dir" \
    --build-type "${build_type^}" \
    --install-dir "$install_dir" \
    --source-dir "$source_dir" \
    --cmake -DBUILD_ACCURACY=OFF \
    --cmake -DBUILD_BENCHMARK=OFF \
    --cmake -DBUILD_TESTS=OFF

bim-package-and-install "$install_dir" fpm "$package_version" "$build_type"
