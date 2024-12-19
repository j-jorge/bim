#!/bin/bash

set -euo pipefail

: "${pack_my_sprites_repository:=https://github.com/j-jorge/pack-my-sprites/}"
: "${pack_my_sprites_version:=1.2.2}"
package_revision=1
version="$pack_my_sprites_version"-"$package_revision"
build_type=release

! bim-install-package pack-my-sprites "$version" "$build_type" 2>/dev/null \
    || exit 0

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

# shellcheck source=SCRIPTDIR/set-package-vars.sh
. "$script_dir"/set-package-vars.sh pack-my-sprites "$build_type"

bim-git-clone-repository \
    "$pack_my_sprites_repository" "$pack_my_sprites_version" "$source_dir"

bim-cmake-build \
    --build-dir "$build_dir" \
    --build-type "$build_type" \
    --install-dir "$install_dir" \
    --source-dir "$source_dir" \
    --cmake -DPMS_BUILD_MAN_PAGES=OFF

bim-package-and-install \
    "$install_dir" pack-my-sprites "$version" "$build_type" boost libclaw
