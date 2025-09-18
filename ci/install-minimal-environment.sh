#!/bin/bash

set -euo pipefail

usage()
{
    cat <<EOF
Install the build dependencies, for those not handled by the internal
package manager.

Usage: $0 OPTIONS

Where OPTIONS are:
  --compiler COMPILER
      Install this C++ compiler (either g++ or clang-N).
  --help, -h
      Display this message and exit.
  --target-platform PLATFORM
      Configure to build for this platform (either android or linux).
EOF
}

while (( $# != 0 ))
do
    arg="$1"
    shift

    case "$arg" in
        --compiler)
            if [[ "$1" = g++ ]] || [[ "$1" == clang-* ]]
            then
                compiler="$1"
            else
                echo "Unknown compiler '$1'." >&2
                exit 1
            fi
            shift
            ;;
        --help)
            usage
            exit 0
            ;;
        --target-platform)
            if [[ "$1" != linux ]] && [[ "$1" != android ]]
            then
                echo "Unknown target platform '$1'." >&2
                exit 1
            fi
            target_platform="$1"
            shift
            ;;
        *)
            echo "Unknown argument '$arg'." >&2
            ;;
    esac
done

[[ -n "${compiler:-}" ]] \
    || (echo "Option --compiler is required." >&2; exit 1)
[[ -n "${target_platform:-}" ]] \
    || (echo "Option --target-platform is required." >&2; exit 1)

function distribution_is()
{
    local expected_id="${1,,}"
    local expected_codename="${2,,}"

    local id
    id="$(grep ^ID= /etc/os-release | cut -d= -f2)"

    local codename
    codename="$(grep ^VERSION_CODENAME= /etc/os-release | cut -d= -f2)"

    [[ "${id,,}" = "$expected_id" ]] \
        && [[ "${codename,,}" = "$expected_codename" ]]
}

function update_alternative_compiler()
{
    local cc_bin="$1"
    local cxx_bin="$2"

    local cc_link="${cc_bin/-*/}"
    local cxx_link="${cxx_bin/-*/}"

    update-alternatives \
        --install /usr/bin/"$cc_link" "$cc_link" /usr/bin/"$cc_bin" 60
    update-alternatives --install /usr/bin/cc cc /usr/bin/"$cc_bin" 60
    update-alternatives --set cc /usr/bin/"$cc_bin"

    update-alternatives \
        --install /usr/bin/"$cxx_link" "$cxx_link" /usr/bin/"$cxx_bin" 60
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/"$cxx_bin" 60
    update-alternatives --set c++ /usr/bin/"$cxx_bin"
}

# Debian Bookworm, which is used for the build images by F-Droid, uses
# a version of GCC too old for Bim!. We are going to install the
# version from Trixie as a workaround.
if [[ "$compiler" = g++ ]] && distribution_is debian bookworm
then
    echo "deb https://deb.debian.org/debian trixie main" \
         > /etc/apt/sources.list.d/trixie.list
    apt-get update
    apt-get install -y -t trixie g++-14

    update_alternative_compiler gcc-14 g++-14

    update_needed=0
else
    packages=("$compiler")
    update_needed=1
fi

packages+=(autoconf
           automake
           bzip2
           ccache
           cmake
           curl
           file
           gettext
           git
           libjpeg-dev
           libpng-dev
           libtool
           make
           ninja-build
           patch
           pkg-config
           python3-venv
          )

# Tracy
packages+=(
    libegl-dev
    libdbus-1-dev
    libxkbcommon-dev
    libwayland-cursor++1
    libwayland-dev
    libwayland-egl++1
)

if [[ "$compiler" == clang-* ]]
then
    clang_version="${compiler/*-/}"
    packages+=("llvm-$clang_version")
fi

case "$target_platform" in
    linux)
        packages+=(libgtk-3-dev
                   mold
                   patch
                   valgrind)
        ;;
    android)
        packages+=(gradle)
        ;;
esac

if ((update_needed == 1))
then
    apt-get update
fi

DEBIAN_FRONTEND=noninteractive \
    apt-get install --no-install-recommends --yes "${packages[@]}"

if [[ "$compiler" == clang-* ]]
then
    update_alternative_compiler \
        clang-"$clang_version" clang++-"$clang_version"
fi


