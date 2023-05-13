#!/bin/bash

set -euo pipefail

: "${iscool_core_repository:=https://github.com/j-jorge/iscool-core/}"
: "${iscool_core_version:=1.8.0}"
package_revision=1
version="$iscool_core_version"-"$package_revision"
build_type=release

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
. "$script_dir"/packaging.sh

! install_package iscool-core "$version" "$build_type" 2>/dev/null \
    || exit 0

source_dir="$bomb_packages_root"/iscool_core/source
build_dir="$bomb_packages_root"/iscool_core/build-"$build_type"
install_dir="$bomb_packages_root"/iscool_core/install-"$build_type"

mkdir --parents "$source_dir" "$build_dir" "$install_dir"

git_clone_repository \
    "$iscool_core_repository" "$iscool_core_version" "$source_dir"

cd "$build_dir"
cmake "$source_dir/build-scripts/cmake" \
      -DCMAKE_BUILD_TYPE="${build_type^}" \
      -DCMAKE_INSTALL_PREFIX="$install_dir" \
      -DCMAKE_PREFIX_PATH="$bomb_app_prefix" \
      -DUSE_DEFAULT_BOOST=ON \
      -DUSE_DEFAULT_JSONCPP=ON \
      -DUSE_DEFAULT_MO_FILE_READER=ON \
      -DISCOOL_CORE_WITH_APPS=ON \
      -DISCOOL_CORE_WITH_CMAKE_PACKAGE=ON \
      -DISCOOL_TEST_ENABLED=OFF

cmake --build . --target install --parallel

package_and_install \
    "$install_dir" iscool-core "$version" "$build_type" \
    mo-file-reader jsoncpp boost
