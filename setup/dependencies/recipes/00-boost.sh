#!/bin/bash

set -euo pipefail

: "${bim_host_prefix:-}"

boost_version=1.87.0
boost_version_underscore="${boost_version//./_}"
package_revision=2
version="$boost_version"-"$package_revision"
build_type=release

! bim-install-package boost "$version" "$build_type" 2>/dev/null \
    || exit 0

archive_basename=boost_"${boost_version_underscore}"
archive_name="${archive_basename}".tar.bz2
archive_url="https://archives.boost.io/release/${boost_version}/source/${archive_name}"

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

# shellcheck source=SCRIPTDIR/set-package-vars.sh
. "$script_dir"/set-package-vars.sh boost "$build_type"

rm --force --recursive "$install_dir"
mkdir --parents "$source_dir" "$build_dir" "$install_dir"

configure()
{
    local libraries
    libraries="$(echo "$@" | tr ' ' ',')"

    ./bootstrap.sh \
        cxxflags="-std=c++20" \
        --with-libraries="$libraries"
}

build()
{
    ./b2 install \
         link=static \
         cxxflags="-std=c++20" \
         -j"$(nproc)" \
         -d0 \
         --prefix="$install_dir" \
         "$@"

    bim-remove-install-dir-from-scripts "$install_dir"
}

build_linux()
{
    build --build-dir="$build_dir"
}

build_android()
{
    local ndk_root
    ndk_root="$(bim-android-config --prefix "$bim_host_prefix" --ndk-root)"

    bim-android-config --arch \
        | while read -r arch
    do
        echo "== Building for $arch =="

        local arch_build_dir="$build_dir"/"$arch"
        rm --force --recursive "$arch_build_dir"
        mkdir --parents "$arch_build_dir"

        local triplet
        triplet="$(bim-android-config --triplet "$arch")"

        local user_config="$arch_build_dir"/user-config.jam
        cat > "$user_config" <<EOF
using clang : android
  : ccache $ndk_root/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++
    --target=$triplet
    --sysroot=$ndk_root/toolchains/llvm/prebuilt/linux-x86_64/sysroot ;
EOF

        build --build-dir="$arch_build_dir" \
              --user-config="$user_config" \
              toolset=clang-android \
              target-os=android

        bim-android-install-lib-in-arch "$install_dir" "$arch"
    done

    bim-clean-up-android-installed-libs "$install_dir"

    find "$install_dir/lib" \
         -name "*.cmake" \
         -exec sed 's/^\(get_filename_component(_BOOST_INCLUDEDIR.\+\)\/include\(.\+\)/\1\/..\/include\2/' -i '{}' \;
}

cd "$source_dir"
download \
    --url="$archive_url" \
    --target-file="$archive_name" \
    --mime-type=application/x-bzip2

tar --bzip2 -xf "$archive_name"
cd "$archive_basename"

case "$bim_package_install_platform" in
    linux)
        configure "program_options,system"
        build --prefix="$install_dir"
        ;;
    android)
        configure "system"
        build_android
        ;;
esac

bim-package-and-install "$install_dir" boost "$version" "$build_type"
