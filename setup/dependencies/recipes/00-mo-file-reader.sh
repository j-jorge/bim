#!/bin/bash

set -euo pipefail

: "${mo_file_reader_repository:=https://github.com/j-jorge/mofilereader/}"
: "${mo_file_reader_version:=1.2}"
package_revision=4
package_version="$mo_file_reader_version"-"$package_revision"
build_type=release

! bim-install-package \
  mo-file-reader "$package_version" "$build_type" 2>/dev/null \
    || exit 0

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

# shellcheck source=SCRIPTDIR/set-package-vars.sh
. "$script_dir"/set-package-vars.sh mo-file-reader "$build_type"

bim-git-clone-repository \
    "$mo_file_reader_repository" "v$mo_file_reader_version" "$source_dir"

bim-cmake-build \
    --build-dir "$build_dir" \
    --build-type "${build_type^}" \
    --install-dir "$install_dir" \
    --source-dir "$source_dir"/build \
    --cmake -DCOMPILE_APP=OFF \
    --cmake -DCOMPILE_DLL=OFF

bim-package-and-install \
    "$install_dir" mo-file-reader "$package_version" "$build_type"
