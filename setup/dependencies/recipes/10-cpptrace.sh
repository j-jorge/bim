#!/bin/bash

set -euo pipefail

[[ "${bim_target_platform:-}" == "linux" ]] || exit 0

: "${cpptrace_repository:=https://github.com/j-jorge/cpptrace}"
: "${cpptrace_version:=1.0.2}"
package_revision=5
version="$cpptrace_version"-"$package_revision"
build_type=release

! bim-install-package cpptrace "$version" "$build_type" 2>/dev/null \
    || exit 0

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

# shellcheck source=SCRIPTDIR/set-package-vars.sh
. "$script_dir"/set-package-vars.sh cpptrace "$build_type"

bim-git-clone-repository \
    "$cpptrace_repository" "v$cpptrace_version" "$source_dir"

bim-cmake-build \
        --build-dir "$build_dir" \
        --build-type "${build_type^}" \
        --install-dir "$install_dir" \
        --source-dir "$source_dir" \
        --cmake -DCPPTRACE_UNWIND_WITH_LIBUNWIND=ON \
        --cmake -DENABLE_DECOMPRESSION=OFF

# The CMake scripts from cpptrace store the full path to the directory
# containing libunwind, making it non-relocatable. We remove this path
# here to fix that.

# There's no substitution here.
# shellcheck disable=SC2016
sed 's,set_target_properties(cpptrace::.\+,find_library(libunwind_path unwind REQUIRED)\nmessage(STATUS "libunwind: ${libunwind_path}")\n&,' \
    -i \
    "${install_dir}/lib/cmake/cpptrace/cpptrace-targets.cmake"

# There's no substitution here.
# shellcheck disable=SC2016
sed 's,;-L/[^>]\+unwind>,;\\$<LINK_ONLY:${libunwind_path}>,' \
    -i \
    "${install_dir}/lib/cmake/cpptrace/cpptrace-targets.cmake"

bim-package-and-install \
    "$install_dir" cpptrace "$version" "$build_type" libunwind
