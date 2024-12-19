#!/bin/bash

set -euo pipefail

: "${jsoncpp_repository:=https://github.com/open-source-parsers/jsoncpp.git}"
: "${jsoncpp_version:=1.9.6}"
package_revision=1
version="$jsoncpp_version"-"$package_revision"
build_type=release

! bim-install-package jsoncpp "$version" "$build_type" 2>/dev/null \
    || exit 0

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

# shellcheck source=SCRIPTDIR/set-package-vars.sh
. "$script_dir"/set-package-vars.sh jsoncpp "$build_type"

bim-git-clone-repository \
    "$jsoncpp_repository" "$jsoncpp_version" "$source_dir"

bim-cmake-build \
        --build-dir "$build_dir" \
        --build-type "${build_type^}" \
        --install-dir "$install_dir" \
        --source-dir "$source_dir" \
        --cmake -DJSONCPP_WITH_CMAKE_PACKAGE=ON \
        --cmake -DJSONCPP_WITH_PKGCONFIG_SUPPORT=OFF \
        --cmake -DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF \
        --cmake -DJSONCPP_WITH_TESTS=OFF \
        --cmake -DBUILD_OBJECT_LIBS=OFF \
        --cmake -DBUILD_SHARED_LIBS=OFF \
        --cmake -DBUILD_STATIC_LIBS=ON

find "$install_dir"/lib \
     -name "*.cmake" \
     -exec sed 's/jsoncpp_static/jsoncpp/g' -i '{}' \;

bim-package-and-install "$install_dir" jsoncpp "$version" "$build_type"
