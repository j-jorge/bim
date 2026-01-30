#!/bin/bash

set -euo pipefail

: "${libclaw_repository:=https://github.com/j-jorge/libclaw/}"
: "${libclaw_version:=1.9.1}"
package_revision=7
version="$libclaw_version"-"$package_revision"
build_type=release

! bim-install-package libclaw "$version" "$build_type" 2>/dev/null \
    || exit 0

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

# shellcheck source=SCRIPTDIR/set-package-vars.sh
. "$script_dir"/set-package-vars.sh libclaw "$build_type"

bim-git-clone-repository \
    "$libclaw_repository" "$libclaw_version" "$source_dir"

bim-cmake-build \
        --build-dir "$build_dir" \
        --build-type "${build_type^}" \
        --install-dir "$install_dir" \
        --source-dir "$source_dir"/cmake \
        --cmake -DCLAW_BUILD_LOCALIZATION=OFF \
        --cmake -DCLAW_BUILD_DOCUMENTATION=OFF \
        --cmake -DCLAW_BUILD_EXAMPLES=OFF

bim-package-and-install "$install_dir" libclaw "$version" "$build_type"
