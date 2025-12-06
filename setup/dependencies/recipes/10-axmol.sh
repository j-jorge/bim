#!/bin/bash

set -euo pipefail

: "${bim_build_type:-}"
: "${bim_product_mode:-}"
: "${bim_package_install_platform:-}"
: "${bim_packages_root:-}"

: "${axmol_repository:=https://github.com/j-jorge/axmol/}"
: "${axmol_version:=2.10.0.1j}"
package_revision=3
version="$axmol_version"-"$package_revision"
flavor="$bim_build_type"

package_name=axmol
package_conflict=axmol-tracy
enable_tracy=0

case "$bim_build_type" in
    asan)
        build_type=RelWithDebInfo
        cmake_options=("--cmake"
                       "-DCMAKE_CXX_FLAGS=-fsanitize=address \
                            -fsanitize=undefined"
                      )
        ;;
    debug)
        build_type=Debug
        ;;
    release)
        build_type=Release

        if [[ "$bim_product_mode" = 0 ]]
        then
            enable_tracy=1
            package_name=axmol-tracy
            package_conflict=axmol
        fi
        ;;
    tsan)
        build_type=RelWithDebInfo
        cmake_options=("--cmake"
                       "-DCMAKE_CXX_FLAGS=-fsanitize=thread")
        ;;
esac

bim-uninstall-package "$package_conflict"

! bim-install-package "$package_name" "$version" "$flavor" 2>/dev/null \
    || exit 0

axmol_definitions=(
    "AX_ENABLE_PREMULTIPLIED_ALPHA=1"
    "AX_ENABLE_SCRIPT_BINDING=0"
    "AX_USE_3D_PHYSICS=0"
    "AX_USE_GL=1"
    "AX_USE_NAVMESH=0"
    "AX_USE_PHYSICS=0"
    "AX_USE_TIFF=0"
    "AX_USE_WEBP=0"
    "AX_USE_WIC=0"
)

axmol_link_libraries=(
    "astcenc"
    "clipper2"
    "ConvertUTF"
    "fmt::fmt"
    "freetype"
    "glad"
    "jpeg"
    "llhttp"
    "openal"
    "ogg"
    "png"
    "pugixml"
    "simdjson"
    "unzip"
    "yasio"
    "xxhash"
    "z"
    "ssl"
    "crypto"
)

if [[ "$bim_package_install_platform" = "android" ]]
then
    ax_gles_profile=200
    ax_use_gl=0

    axmol_definitions+=(
        "ANDROID=1"
        "AX_GLES_PROFILE=$ax_gles_profile"
        "AL_LIBTYPE_STATIC=1"
    )
    axmol_link_libraries+=(
        "GLESv2"
        "EGL"
        "log"
        "android"
        "OpenSLES"
    )
else
    ax_gles_profile=""
    ax_use_gl=1

    axmol_definitions+=("LINUX=1")
    declare -a gtk_libs < <(pkg-config --libs gtk+-3.0 | sed 's/-l//g')
    axmol_link_libraries+=(
        "${gtk_libs[@]}"
        "fontconfig"
        "glfw"
        "GL"
        "X11"
        "dl"
        "pthread"
    )
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

# shellcheck source=SCRIPTDIR/set-package-vars.sh
. "$script_dir"/set-package-vars.sh axmol "$flavor"

rm --force --recursive "$install_dir"
mkdir --parents "$source_dir" "$build_dir" "$install_dir"

bim-git-clone-repository \
    "$axmol_repository" v"$axmol_version" "$source_dir"

install_power_shell()
{
    local local_root="$bim_packages_root"/axmol/powershell
    rm --force --recursive "$local_root"
    mkdir --parents "$local_root"
    cd "$local_root"

    local archive_name=powershell-7.4.1-linux-x64.tar.gz
    download \
    --url="https://github.com/PowerShell/PowerShell/releases/download/v7.4.1/$archive_name" \
    --target-file="$archive_name" \
    --mime-type=application/gzip

    tar -xf "$archive_name"
    PATH="$(pwd):$PATH"
    export PATH
}

build()
{
    local cmake_options=("--cmake" "-Daxmol_root=$source_dir")

    for definition in "${axmol_definitions[@]}"
    do
        cmake_options+=("--cmake" "-D$definition")
    done

    cmake_options+=("--cmake" "-DAX_ENABLE_TRACY=${enable_tracy}")

    # Axmol's dependency management tool, 1k, is implemented in .Net,
    # which requires libicu unless told not to. This environment variable
    # turns this requirement off.
    export DOTNET_SYSTEM_GLOBALIZATION_INVARIANT=1

    bim-cmake-build \
        --build-dir "$build_dir" \
        --build-type "${build_type^}" \
        --install-dir "$install_dir" \
        --source-dir "$script_dir"/axmol \
        "${cmake_options[@]}"
}

fix_up_openal_headers()
{
    # Axmol explicitly includes OpenAL headers from its own 3rdparty
    # directory, which won't work with our installed
    # package. Consequently we adjust the source and the installation
    # to make it independent from Axmol's source tree.
    cp --recursive \
       "$source_dir"/3rdparty/openal/include/AL \
       "$install_dir"/include/

    sed 's,3rdparty/openal/include/,,' \
        -i \
        "$install_dir"/include/axmol/audio/oal_port.h
}

clean_up_install()
{
    local axmol_include="${install_dir:?}"/include/axmol

    find "$axmol_include" \
         \( \
         -name "*-apple.*" \
         -o -name "*-ios.*" \
         -o -name "*-mac.*" \
         -o -name "*-win32.*" \
         -o -name "*-winrt.*" \
         \) \
         -delete

    rm --force --recursive \
       "${axmol_include:?}"/cocos2d.h \
       "${axmol_include:?}"/media \
       "${axmol_include:?}"/navmesh \
       "${axmol_include:?}"/physics \
       "${axmol_include:?}"/physics3d \
       "${axmol_include:?}"/platform/apple \
       "${axmol_include:?}"/platform/ios \
       "${axmol_include:?}"/platform/mac \
       "${axmol_include:?}"/platform/win32 \
       "${axmol_include:?}"/platform/winrt

    if [[ "$bim_package_install_platform" = "android" ]]
    then
        find "$axmol_include" \
             -name "*-linux.*" \
             -delete
        rm --force --recursive "${axmol_include:?}"/platform/linux
    else
        find "$axmol_include" \
             -name "*-android.*" \
             -delete
        rm --force --recursive "${axmol_include:?}"/platform/android
    fi
}

install_cmake_file()
{
    local cmake_module_dir="$1"

    mkdir --parents "$cmake_module_dir"

    cp "$source_dir"/cmake/Modules/AXSLCC.cmake "$cmake_module_dir"

    cat > "$cmake_module_dir"/axmol-config.cmake <<EOF
if(TARGET axmol::axmol)
  return()
endif()

message(STATUS "Axmol with Tracy: \${AX_ENABLE_TRACY}")

if (AX_ENABLE_TRACY)
  set(AX_ENABLE_TRACY 1)
  find_package(Tracy REQUIRED)
else()
  set(AX_ENABLE_TRACY 0)
endif()

set(AX_GLES_PROFILE $ax_gles_profile)
set(AX_USE_GL $ax_use_gl)
include(\${CMAKE_CURRENT_LIST_DIR}/AXSLCC.cmake)

find_path(
  axmol_include_dir
  NAMES axmol.h
  PATH_SUFFIXES axmol
)

set(
  axmol_definitions
$(printf "  %s\n" "${axmol_definitions[@]}")
  AX_ENABLE_TRACY=\${AX_ENABLE_TRACY}
)

function(link_axmol_library name)
  unset(axmol_dependency CACHE)
  string(FIND "\${name}" "::" colon_colon)

  if(\${colon_colon} EQUAL -1)
    find_library(axmol_dependency NAMES "\${name}" REQUIRED)
  else()
    string(SUBSTRING "\${name}" 0 \${colon_colon} package_name)
    find_package("\${package_name}" REQUIRED)
    set(axmol_dependency "\${name}")
  endif()

  set(axmol_libraries "\${axmol_libraries}" "\${axmol_dependency}" PARENT_SCOPE)
endfunction()

if(AX_ENABLE_TRACY)
  link_axmol_library(axmol_tracy)
else()
  link_axmol_library(axmol)
endif()

$(printf "link_axmol_library(%s)\n" "${axmol_link_libraries[@]}")

add_library(axmol::axmol INTERFACE IMPORTED)

cmake_path(GET axmol_include_dir PARENT_PATH axmol_parent_include_dir)

set(axmol_include_directories
  "\${axmol_include_dir}"
  "\${axmol_parent_include_dir}"
)

if(\${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  list(APPEND axmol_include_directories "\${axmol_parent_include_dir}/GLFW")
endif()

set_target_properties(
  axmol::axmol
  PROPERTIES
  INTERFACE_COMPILE_DEFINITIONS "\${axmol_definitions}"
  INTERFACE_INCLUDE_DIRECTORIES "\${axmol_include_directories}"
  INTERFACE_LINK_LIBRARIES "\${axmol_libraries}"
)

cmake_path(GET axmol_parent_include_dir PARENT_PATH axmol_system_root)

file(GLOB axmol_shader_files \${axmol_system_root}/share/axmol/shaders/*)
EOF

    cat > "$cmake_module_dir/axmol-config-version.cmake" <<EOF
set(PACKAGE_VERSION "$axmol_version")

if(PACKAGE_VERSION VERSION_LESS PACKAGE_FIND_VERSION)
  set(PACKAGE_VERSION_COMPATIBLE FALSE)
else()
  set(PACKAGE_VERSION_COMPATIBLE TRUE)

  if(PACKAGE_FIND_VERSION STREQUAL PACKAGE_VERSION)
      set(PACKAGE_VERSION_EXACT TRUE)
  endif()
endif()
EOF
}

install_power_shell
build
fix_up_openal_headers
clean_up_install

if [[ "$bim_package_install_platform" = "android" ]]
then
    java_build_dir="$bim_packages_root"/axmol/java-build-"$build_type"

    bim-android-java-build \
        --build-dir "$java_build_dir" \
        --build-type "$build_type" \
        --artifact-group "axmol" \
        --artifact-id "axmol" \
        --artifact-version "$version" \
        --install-dir "$install_dir" \
        --namespace 'dev.axmol.lib' \
        --api-dependency "androidx.media3:media3-exoplayer:1.0.2" \
        --implementation-dependency "androidx.annotation:annotation:1.3.0" \
        --source-dir "$source_dir"/core/platform/android/java/

    bim-android-config --arch \
        | while read -r arch
    do
        triplet="$(bim-android-config --triplet "$arch")"
        install_cmake_file "$install_dir"/lib/"$triplet"/cmake/axmol/
    done
else
    install_cmake_file "$install_dir"/lib/cmake/axmol/
fi

bim-package-and-install \
    "$install_dir" "$package_name" "$version" "$flavor"
