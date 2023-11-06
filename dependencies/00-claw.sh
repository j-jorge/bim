#!/bin/bash

set -euo pipefail

: "${libclaw_repository:=https://github.com/j-jorge/libclaw/}"
: "${libclaw_version:=1.8.1}"
package_revision=1
version="$libclaw_version"-"$package_revision"
build_type=release

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
. "$script_dir"/packaging.sh

! install_package libclaw "$version" "$build_type" 2>/dev/null \
    || exit 0

source_dir="$bomb_packages_root"/libclaw/source
build_dir="$bomb_packages_root"/libclaw/build-"$build_type"
install_dir="$bomb_packages_root"/libclaw/install-"$build_type"

mkdir --parents "$source_dir" "$build_dir" "$install_dir"

git_clone_repository \
    "$libclaw_repository" "$libclaw_version" "$source_dir"

cd "$build_dir"
cmake "$source_dir"/cmake \
      -DCMAKE_BUILD_TYPE="${build_type^}" \
      -DCMAKE_INSTALL_PREFIX="$install_dir"
cmake --build . --target install --parallel

package_and_install "$install_dir" libclaw "$version" "$build_type"
