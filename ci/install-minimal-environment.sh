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

packages=(autoconf
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
          pkg-config
          python3-venv
          "$compiler")

if [[ "$compiler" == clang-* ]]
then
    clang_version="${compiler/*-/}"
    packages+=("llvm-$clang_version")
fi

case "$target_platform" in
    linux)
        packages+=(libgtk-3-dev
                   mold
                   valgrind)
        ;;
    android)
        packages+=(gradle)
        ;;
esac

apt-get update
DEBIAN_FRONTEND=noninteractive \
    apt-get install --no-install-recommends --yes "${packages[@]}"

if [[ "$compiler" == clang-* ]]
then
    update-alternatives \
        --install /usr/bin/clang clang \
        /usr/bin/clang-"$clang_version" 60
    update-alternatives \
        --install /usr/bin/cc cc /usr/bin/clang-"$clang_version" 60
    update-alternatives --set cc /usr/bin/clang-"$clang_version"

    update-alternatives \
        --install /usr/bin/clang++ clang++ \
        /usr/bin/clang++-"$clang_version" 60
    update-alternatives \
        --install /usr/bin/c++ c++ /usr/bin/clang++-"$clang_version" 60
    update-alternatives --set c++ /usr/bin/clang++-"$clang_version"
fi


