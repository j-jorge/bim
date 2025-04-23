#!/bin/bash

set -euo pipefail

[[ "${bim_target_platform:-}" == "linux" ]] || exit 0

: "${maxminddb_repository:=https://github.com/maxmind/libmaxminddb}"
: "${maxminddb_version:=1.12.2}"
package_revision=1
version="$maxminddb_version"-"$package_revision"
build_type=release

! bim-install-package maxminddb "$version" "$build_type" 2>/dev/null \
    || exit 0

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

# shellcheck source=SCRIPTDIR/set-package-vars.sh
. "$script_dir"/set-package-vars.sh maxminddb "$build_type"

bim-git-clone-repository \
    "$maxminddb_repository" "$maxminddb_version" "$source_dir"

bim-cmake-build \
        --build-dir "$build_dir" \
        --build-type "${build_type^}" \
        --install-dir "$install_dir" \
        --source-dir "$source_dir" \
        --cmake -DBUILD_TESTING=OFF \
        --cmake -DMAXMIND_BUILD_BINARIES=OFF

bim-package-and-install "$install_dir" maxminddb "$version" "$build_type"
