#!/bin/bash

set -euo pipefail

: "${bim_product_mode:-}"
: "${tracy_repository:=https://github.com/wolfpld/tracy}"
: "${tracy_version:=0.12.2}"

# Tracy is a tool for the developers, we don't use it when building
# the final product.
[[ "$bim_product_mode" = 0 ]] || exit 0

package_revision=3
version="$tracy_version"-"$package_revision"
build_type=release

! bim-install-package tracy "$version" "$build_type" 2>/dev/null \
    || exit 0

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

# shellcheck source=SCRIPTDIR/set-package-vars.sh
. "$script_dir"/set-package-vars.sh tracy "$build_type"

bim-git-clone-repository \
    "$tracy_repository" "v$tracy_version" "$source_dir"

bim-cmake-build \
        --build-dir "$build_dir" \
        --build-type "${build_type^}" \
        --install-dir "$install_dir" \
        --source-dir "$source_dir" \
        --cmake -DTRACY_STATIC=ON \
        --cmake -DTRACY_DELAYED_INIT=ON \
        --cmake -DTRACY_LTO=OFF

bim-package-and-install \
    "$install_dir" tracy "$version" "$build_type"
