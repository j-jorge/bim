#!/bin/bash

set -euo pipefail

: "${fmt_repository:=https://github.com/fmtlib/fmt.git/}"
: "${fmt_version:=10.2.1}"
package_revision=1
package_version="$fmt_version"-"$package_revision"
build_type=release

! bim-install-package fmt "$package_version" "$build_type" 2>/dev/null \
    || exit 0

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

# shellcheck source=SCRIPTDIR/set-package-vars.sh
. "$script_dir"/set-package-vars.sh fmt "$build_type"

bim-git-clone-repository "$fmt_repository" "$fmt_version" "$source_dir"

bim-cmake-build \
    --build-dir "$build_dir" \
    --build-type "${build_type^}" \
    --install-dir "$install_dir" \
    --source-dir "$source_dir" \
    --cmake -DFMT_TEST=OFF

bim-package-and-install "$install_dir" fmt "$package_version" "$build_type"
