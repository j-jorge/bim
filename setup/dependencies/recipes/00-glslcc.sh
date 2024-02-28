#!/bin/bash

set -euo pipefail

: "${bim_packages_root:-}"

glslcc_version=1.9.3
package_revision=1
version="$glslcc_version"-"$package_revision"
build_type=release

! bim-install-package glslcc "$version" "$build_type" 2>/dev/null \
    || exit 0

archive_name=glslcc-"${glslcc_version}"-linux.tar.gz
archive_url="https://github.com/axmolengine/glslcc/releases/download/v${glslcc_version}/${archive_name}"

source_dir="$bim_packages_root"/glslcc/source
install_dir="$bim_packages_root"/glslcc/install-"$build_type"

mkdir --parents "$source_dir" "$install_dir"

cd "$source_dir"

download \
    --url="$archive_url" \
    --target-file="$archive_name" \
    --mime-type=application/gzip

tar -xf "$archive_name"

mkdir "$install_dir"/bin
mv glslcc "$install_dir"/bin

bim-package-and-install "$install_dir" glslcc "$version" "$build_type"
