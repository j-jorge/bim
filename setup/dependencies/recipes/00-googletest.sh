#!/bin/bash

set -euo pipefail

: "${googletest_repository:=https://github.com/google/googletest/}"
: "${googletest_version:=1.15.2}"
package_revision=3
version="$googletest_version"-"$package_revision"
build_type=release

! bim-install-package googletest "$version" "$build_type" 2>/dev/null \
    || exit 0

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

# shellcheck source=SCRIPTDIR/set-package-vars.sh
. "$script_dir"/set-package-vars.sh googletest "$build_type"

bim-git-clone-repository \
    "$googletest_repository" "v$googletest_version" "$source_dir"

bim-cmake-build \
        --build-dir "$build_dir" \
        --build-type "${build_type^}" \
        --install-dir "$install_dir" \
        --source-dir "$source_dir"

bim-package-and-install "$install_dir" googletest "$version" "$build_type"
