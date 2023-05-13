#!/bin/bash

set -euo pipefail

: "${mo_file_reader_repository:=https://github.com/j-jorge/mofilereader/}"
: "${mo_file_reader_version:=1.1}"
package_revision=1
package_version="$mo_file_reader_version"-"$package_revision"
build_type=release

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
. "$script_dir"/packaging.sh

! install_package mo-file-reader "$package_version" "$build_type" 2>/dev/null \
    || exit 0

source_dir="$bomb_packages_root"/mo-file-reader/source
build_dir="$bomb_packages_root"/mo-file-reader/build-"$build_type"
install_dir="$bomb_packages_root"/mo-file-reader/install-"$build_type"

mkdir --parents "$source_dir" "$build_dir" "$install_dir"

git_clone_repository \
    "$mo_file_reader_repository" "v$mo_file_reader_version" "$source_dir"

cd "$build_dir"
cmake "$source_dir"/build \
      -DCMAKE_BUILD_TYPE="${build_type^}" \
      -DCMAKE_INSTALL_PREFIX="$install_dir" \
      -DCOMPILE_APP=OFF \
      -DCOMPILE_DLL=OFF
cmake --build . --target install --parallel

package_and_install \
    "$install_dir" mo-file-reader "$package_version" "$build_type"
