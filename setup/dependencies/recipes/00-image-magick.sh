#!/bin/bash

set -euo pipefail

: "${image_magick_repository:=https://github.com/ImageMagick/ImageMagick/}"
: "${image_magick_version:=7.1.2-13}"
package_revision=1
version="$image_magick_version"-"$package_revision"
build_type=release

! bim-install-package image-magick "$version" "$build_type" 2>/dev/null \
    || exit 0

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

# shellcheck source=SCRIPTDIR/set-package-vars.sh
. "$script_dir"/set-package-vars.sh image-magick "$build_type"

bim-git-clone-repository \
    "$image_magick_repository" "$image_magick_version" "$source_dir"

rm --force --recursive "$build_dir"
mkdir --parents "$build_dir"
cd "$build_dir"

"$source_dir"/configure \
             --disable-dependency-tracking \
             --disable-docs \
             --disable-openmp \
             --disable-shared \
             --disable-static \
             --enable-zero-configuration \
             --prefix "$install_dir" \
             --without-bzlib \
             --without-djvu \
             --without-dmr \
             --without-fontconfig \
             --without-freetype \
             --without-gdi32 \
             --without-heic \
             --without-jbig \
             --without-jxl \
             --without-lcms \
             --without-lqr \
             --without-lzma \
             --without-magick-plus-plus \
             --without-openexr \
             --without-openjp2 \
             --without-pango \
             --without-raqm \
             --without-raw \
             --without-tiff \
             --without-webp \
             --without-x \
             --without-xml \
             --without-zip \
             --without-zstd

make -j "$(nproc)" install

rm --force --recursive \
   "${install_dir:?}"/{etc,include,lib,share} \
   "${install_dir:?}"/bin/Magick*

bim-package-and-install "$install_dir" image-magick "$version" "$build_type"
