#!/bin/bash

set -euo pipefail

: "${pack_my_sprites_repository:=https://github.com/j-jorge/pack-my-sprites/}"
: "${pack_my_sprites_version:=1.0.2}"
package_revision=1
version="$pack_my_sprites_version"-"$package_revision"
build_type=release

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
. "$script_dir"/packaging.sh

! install_package pack-my-sprites "$version" "$build_type" 2>/dev/null \
    || exit 0

source_dir="$bomb_packages_root"/pack-my-sprites/source
build_dir="$bomb_packages_root"/pack-my-sprites/build-"$build_type"
install_dir="$bomb_packages_root"/pack-my-sprites/install-"$build_type"

mkdir --parents "$source_dir" "$build_dir" "$install_dir"

git_clone_repository \
    "$pack_my_sprites_repository" "$pack_my_sprites_version" "$source_dir"

cd "$build_dir"
cmake "$source_dir" \
      -DCMAKE_BUILD_TYPE="${build_type^}" \
      -DCMAKE_INSTALL_PREFIX="$install_dir" \
      -DCMAKE_PREFIX_PATH="$bomb_app_prefix" \
      -DPMS_BUILD_MAN_PAGES=OFF
cmake --build . --target install --parallel

package_and_install \
    "$install_dir" pack-my-sprites "$version" "$build_type" boost libclaw
