#!/bin/bash

set -euo pipefail

: "${tracy_repository:=https://github.com/wolfpld/tracy}"
: "${tracy_version:=0.12.2}"
package_revision=1
version="$tracy_version"-"$package_revision"
build_type=release

! bim-install-package tracy-profiler "$version" "$build_type" 2>/dev/null \
    || exit 0

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

# shellcheck source=SCRIPTDIR/set-package-vars.sh
. "$script_dir"/set-package-vars.sh tracy "$build_type"

bim-git-clone-repository \
    "$tracy_repository" "v$tracy_version" "$source_dir"

bim-cmake-build \
    --build-dir "$build_dir-profiler" \
    --build-type "${build_type^}" \
    --install-dir "$install_dir" \
    --source-dir "$source_dir/profiler" \
    --no-lto

bim-package-and-install \
    "$install_dir" tracy-profiler "$version" "$build_type"
