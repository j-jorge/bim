#!/bin/bash

set -euo pipefail

boost_version=1.82.0
boost_version_underscore="${boost_version//./_}"
package_revision=2
version="$boost_version"-"$package_revision"
build_type=release

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
. "$script_dir"/packaging.sh

iscool_include_root=$(iscool-shell-config --shell-include)

. "$iscool_include_root"/mime-types.sh
. "$iscool_include_root"/platform.sh

! install_package boost "$version" "$build_type" 2>/dev/null \
    || exit 0

libraries=("program_options" "system")

archive_basename=boost_"${boost_version_underscore}"
archive_name="${archive_basename}".tar.bz2
archive_url="https://boostorg.jfrog.io/artifactory/main/release/${boost_version}/source/${archive_name}"

source_dir="$bomb_packages_root"/boost/source
build_dir="$bomb_packages_root"/boost/build-"$build_type"
install_dir="$bomb_packages_root"/boost/install-"$build_type"

mkdir --parents "$source_dir" "$build_dir" "$install_dir"

cd "$source_dir"

download \
    --url="$archive_url" \
    --target-file="$archive_name" \
    --mime-type=application/x-bzip2

tar --bzip2 -xf "$archive_name"
cd "$archive_basename"

./bootstrap.sh \
    cxxflags="-std=c++20" \
    --prefix="$install_dir" \
    --with-libraries="$(echo "${libraries[@]}" | tr ' ' ',')"

./b2 install \
     link=static \
     cxxflags="-std=c++20" \
     linkflags="-ldl" \
     --prefix="$install_dir" \
     -j"$(cpu_count)"

package_and_install "$install_dir" boost "$version" "$build_type"
