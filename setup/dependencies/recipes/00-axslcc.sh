#!/bin/bash

set -euo pipefail

: "${bim_packages_root:-}"

axslcc_version=1.9.6
package_revision=1
version="$axslcc_version"-"$package_revision"
build_type=release

! bim-install-package axslcc "$version" "$build_type" 2>/dev/null \
    || exit 0

archive_name=axslcc-"${axslcc_version}"-linux.tar.gz
archive_url="https://github.com/axmolengine/axslcc/releases/download/v${axslcc_version}/${archive_name}"

source_dir="$bim_packages_root"/axslcc/source
install_dir="$bim_packages_root"/axslcc/install-"$build_type"

mkdir --parents "$source_dir" "$install_dir"

cd "$source_dir"

download \
    --url="$archive_url" \
    --target-file="$archive_name" \
    --mime-type=application/gzip

tar -xf "$archive_name"

mkdir "$install_dir"/bin
mv axslcc "$install_dir"/bin

bim-package-and-install "$install_dir" axslcc "$version" "$build_type"
