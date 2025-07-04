#!/bin/bash

set -euo pipefail

[[ "${bim_target_platform:-}" == "linux" ]] || exit 0

: "${cpptrace_repository:=https://github.com/j-jorge/cpptrace}"
: "${cpptrace_version:=1.0.2}"
package_revision=2
version="$cpptrace_version"-"$package_revision"
build_type=release

! bim-install-package cpptrace "$version" "$build_type" 2>/dev/null \
    || exit 0

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

# shellcheck source=SCRIPTDIR/set-package-vars.sh
. "$script_dir"/set-package-vars.sh cpptrace "$build_type"

bim-git-clone-repository \
    "$cpptrace_repository" "v$cpptrace_version" "$source_dir"

bim-cmake-build \
        --build-dir "$build_dir" \
        --build-type "${build_type^}" \
        --install-dir "$install_dir" \
        --source-dir "$source_dir" \
        --cmake -DCPPTRACE_UNWIND_WITH_LIBUNWIND=ON \
        --cmake -DENABLE_DECOMPRESSION=OFF

bim-package-and-install \
    "$install_dir" cpptrace "$version" "$build_type" libunwind
