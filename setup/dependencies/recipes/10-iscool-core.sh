#!/bin/bash

set -euo pipefail

: "${bim_build_type:-}"

: "${iscool_core_repository:=https://github.com/j-jorge/iscool-core/}"
: "${iscool_core_version:=1.17.0}"
package_revision=1
version="$iscool_core_version"-"$package_revision"
flavor="$bim_build_type"
build_type=
cmake_options=()

case "$bim_build_type" in
    asan)
        build_type=RelWithDebInfo
        cmake_options=("--cmake"
                       "-DCMAKE_CXX_FLAGS=-fsanitize=address \
                            -fsanitize=undefined \
                            -fno-omit-frame-pointer"
                      )
        ;;
    debug)
        build_type=Debug
        ;;
    release)
        build_type=Release
        ;;
    tsan)
        build_type=RelWithDebInfo
        cmake_options=("--cmake"
                       "-DCMAKE_CXX_FLAGS=-fsanitize=thread \
                           -fno-omit-frame-pointer")
        ;;
esac

! bim-install-package iscool-core "$version" "$flavor" 2>/dev/null \
    || exit 0

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

# shellcheck source=SCRIPTDIR/set-package-vars.sh
. "$script_dir"/set-package-vars.sh iscool-core "$flavor"

bim-git-clone-repository \
    "$iscool_core_repository" "$iscool_core_version" "$source_dir"

if [[ "$bim_package_install_platform" == "linux" ]]
then
    cmake_options+=("--cmake" "-DISCOOL_CORE_WITH_APPS=ON")
fi

bim-cmake-build \
    --build-dir "$build_dir" \
    --build-type "${build_type^}" \
    --install-dir "$install_dir" \
    --source-dir "$source_dir"/build-scripts/cmake \
    --cmake -DUSE_DEFAULT_BOOST=ON \
    --cmake -DUSE_DEFAULT_JSONCPP=ON \
    --cmake -DUSE_DEFAULT_MO_FILE_READER=ON \
    --cmake -DISCOOL_CORE_WITH_CMAKE_PACKAGE=ON \
    --cmake -DISCOOL_TEST_ENABLED=OFF \
    --cmake -DISCOOL_DISABLE_MODULES="video;locale" \
    "${cmake_options[@]}"

build_android_lib()
{
    local module="$1"
    shift

    java_build_dir="$bim_packages_root"/iscool-core/java-build-"$build_type"/"$module"

    bim-android-java-build \
        --build-dir "$java_build_dir" \
        --build-type "$build_type" \
        --artifact-group "iscool" \
        --artifact-id "$module" \
        --artifact-version "$version" \
        --install-dir "$install_dir" \
        --namespace iscool."$module" \
        --source-dir "$source_dir"/modules/core/"$module"/android/java/ \
        "$@"
}

if [[ "$bim_package_install_platform" = "android" ]]
then
    build_android_lib \
        social \
        --api-dependency "androidx.annotation:annotation:1.8.0" \
        --api-dependency "androidx.core:core:1.8.0"

    build_android_lib \
        system \
        --api-dependency "androidx.annotation:annotation:1.8.0"
    build_android_lib \
        log \
        --api-dependency "androidx.annotation:annotation:1.8.0"
    build_android_lib jni --implementation-dependency "iscool:log:$version"
fi

bim-package-and-install \
    "$install_dir" iscool-core "$version" "$flavor" \
    mo-file-reader jsoncpp boost
