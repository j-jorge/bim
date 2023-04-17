#!/bin/bash

set -euo pipefail

: "${googletest_repository:=https://github.com/google/googletest/}"
: "${googletest_commit:=v1.13.0}"
package_revision=1
version="$googletest_commit"-"$package_revision"

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
. "$script_dir"/packaging.sh

! install_package googletest "$version" 2>/dev/null \
    || exit 0

source_dir="$bomb_packages_root"/googletest/source
build_dir="$bomb_packages_root"/googletest/build-"$bomb_build_type"
install_dir="$bomb_packages_root"/googletest/install-"$bomb_build_type"

mkdir --parents "$source_dir" "$build_dir" "$install_dir"

git_clone_repository "$googletest_repository" "$googletest_commit" "$source_dir"

cd "$build_dir"
cmake "$source_dir" \
      -DCMAKE_BUILD_TYPE="${bomb_build_type^}" \
      -DCMAKE_INSTALL_PREFIX="$install_dir"
cmake --build . --target install --parallel

package_and_install "$install_dir" googletest "$version"
