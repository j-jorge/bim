#!/bin/bash

set -euo pipefail

: "${iscool_core_repository:=https://github.com/j-jorge/iscool-core/}"
: "${iscool_core_version:=1.8.0}"
package_revision=1
version="$iscool_core_version"-"$package_revision"
flavor="$bomb_build_type"

case "$bomb_build_type" in
    asan)
        cmake_options=("-DCMAKE_BUILD_TYPE=RelWithDebInfo"
                       "-DCMAKE_CXX_FLAGS=-fsanitize=address \
                            -fsanitize=undefined \
                            -fno-omit-frame-pointer"
                      )
        ;;
    debug)
        cmake_options=("-DCMAKE_BUILD_TYPE=Debug")
        ;;
    release)
        cmake_options=("-DCMAKE_BUILD_TYPE=Release")
        ;;
    tsan)
        cmake_options=("-DCMAKE_BUILD_TYPE=RelWithDebInfo"
                       "-DCMAKE_CXX_FLAGS=-fsanitize=thread -fno-omit-frame-pointer")
        ;;
esac

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
. "$script_dir"/packaging.sh

! install_package iscool-core "$version" "$flavor" 2>/dev/null \
    || exit 0

source_dir="$bomb_packages_root"/iscool_core/source
build_dir="$bomb_packages_root"/iscool_core/build-"$flavor"
install_dir="$bomb_packages_root"/iscool_core/install-"$flavor"

mkdir --parents "$source_dir" "$build_dir" "$install_dir"

git_clone_repository \
    "$iscool_core_repository" "$iscool_core_version" "$source_dir"

cd "$build_dir"
cmake "$source_dir/build-scripts/cmake" \
      -DCMAKE_INSTALL_PREFIX="$install_dir" \
      -DCMAKE_PREFIX_PATH="$bomb_app_prefix" \
      -DUSE_DEFAULT_BOOST=ON \
      -DUSE_DEFAULT_JSONCPP=ON \
      -DUSE_DEFAULT_MO_FILE_READER=ON \
      -DISCOOL_CORE_WITH_APPS=ON \
      -DISCOOL_CORE_WITH_CMAKE_PACKAGE=ON \
      -DISCOOL_TEST_ENABLED=OFF \
      "${cmake_options[@]}"

cmake --build . --target install --parallel

package_and_install \
    "$install_dir" iscool-core "$version" "$flavor" \
    mo-file-reader jsoncpp boost
