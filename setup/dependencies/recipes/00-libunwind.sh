#!/bin/bash

set -euo pipefail

[[ "${bim_target_platform:-}" == "linux" ]] || exit 0

: "${libunwind_repository:=https://github.com/j-jorge/libunwind}"
: "${libunwind_version:=1.8.3}"
package_revision=3
version="$libunwind_version"-"$package_revision"
build_type=release

! bim-install-package libunwind "$version" "$build_type" 2>/dev/null \
    || exit 0

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

# shellcheck source=SCRIPTDIR/set-package-vars.sh
. "$script_dir"/set-package-vars.sh libunwind "$build_type"

bim-git-clone-repository \
    "$libunwind_repository" "v$libunwind_version" "$source_dir"

rm --force --recursive "$build_dir"
mkdir --parents "$build_dir"
cd "$build_dir"

autoreconf --install "$source_dir"

"$source_dir"/configure \
             --disable-aix-soname \
             --disable-coredump \
             --disable-cxx-exceptions \
             --disable-debug \
             --disable-dependency-tracking \
             --disable-docs \
             --disable-documentation \
             --disable-minidebuginfo \
             --disable-msabi-support \
             --disable-nto \
             --disable-ptrace \
             --disable-setjmp \
             --disable-shared \
             --disable-tests \
             --disable-weak-backtrace \
             --disable-zlibdebuginfo \
             --enable-fast-install \
             --enable-static \
             --prefix "$install_dir"

make -j "$(nproc)" install

 rm --force --recursive \
    "${install_dir:?}"/libexec \
    "${install_dir:?}"/lib/*-* \
    "${install_dir:?}"/lib/*-* \
    "${install_dir:?}"/lib/*.la

bim-package-and-install "$install_dir" libunwind "$version" "$build_type"
