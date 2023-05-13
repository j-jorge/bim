#!/bin/bash

set -euo pipefail

: "${googletest_repository:=https://github.com/google/googletest/}"
: "${googletest_version:=1.13.0}"
package_revision=1
version="$googletest_version"-"$package_revision"
build_type=release

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
. "$script_dir"/packaging.sh

! install_package googletest "$version" "$build_type" 2>/dev/null \
    || exit 0

source_dir="$bomb_packages_root"/googletest/source
build_dir="$bomb_packages_root"/googletest/build-"$build_type"
install_dir="$bomb_packages_root"/googletest/install-"$build_type"

mkdir --parents "$source_dir" "$build_dir" "$install_dir"

git_clone_repository \
    "$googletest_repository" "v$googletest_version" "$source_dir"

cd "$build_dir"
cmake "$source_dir" \
      -DCMAKE_BUILD_TYPE="${build_type^}" \
      -DCMAKE_INSTALL_PREFIX="$install_dir"
cmake --build . --target install --parallel

package_and_install "$install_dir" googletest "$version" "$build_type"
