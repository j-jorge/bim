#!/bin/bash

set -euo pipefail

: "${jsoncpp_repository:=https://github.com/open-source-parsers/jsoncpp.git}"
: "${jsoncpp_version:=1.8.4}"
package_revision=1
version="$jsoncpp_version"-"$package_revision"
build_type=release

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
. "$script_dir"/packaging.sh

! install_package jsoncpp "$version" "$build_type" 2>/dev/null \
    || exit 0

source_dir="$bomb_packages_root"/jsoncpp/source
build_dir="$bomb_packages_root"/jsoncpp/build-"$build_type"
install_dir="$bomb_packages_root"/jsoncpp/install-"$build_type"

mkdir --parents "$source_dir" "$build_dir" "$install_dir"

git_clone_repository \
    "$jsoncpp_repository" "$jsoncpp_version" "$source_dir"

cd "$build_dir"
cmake "$source_dir" \
      -DCMAKE_BUILD_TYPE="${build_type^}" \
      -DCMAKE_INSTALL_PREFIX="$install_dir" \
      -DJSONCPP_WITH_CMAKE_PACKAGE=ON \
      -DJSONCPP_WITH_PKGCONFIG_SUPPORT=OFF \
      -DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF \
      -DJSONCPP_WITH_TESTS=OFF
cmake --build . --target install --parallel

package_and_install "$install_dir" jsoncpp "$version" "$build_type"
