#!/bin/bash

set -euo pipefail

: "${axmol_repository:=https://github.com/j-jorge/axmol/}"
: "${axmol_version:=2.0.0-j}"
package_revision=1
version="$axmol_version"-"$package_revision"

if [[ "$bomb_build_type" = "release" ]]
then
    build_type=release
else
    build_type=debug
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
. "$script_dir"/packaging.sh

! install_package axmol "$version" "$build_type" 2>/dev/null \
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
    "LINUX=1"
)

axmol_link_libraries=(
    "axmol"
    "astcenc"
    $(pkg-config --libs gtk+-3.0 | sed 's/-l//g')
    "clipper2"
    "ConvertUTF"
    "freetype"
    "glad"
    "glfw"
    "jpeg"
    "llhttp"
    "openal"
    "ogg"
    "png"
    "pugixml"
    "unzip"
    "xxhash"
    "z"
    "ssl"
    "crypto"
    "fontconfig"
    "GL"
    "X11"
    "dl"
    "pthread"
)

source_dir="$bomb_packages_root"/axmol/source
build_dir="$bomb_packages_root"/axmol/build-"$build_type"
install_dir="$bomb_packages_root"/axmol/install-"$build_type"

rm --force --recursive "$install_dir"
mkdir --parents "$source_dir" "$build_dir" "$install_dir"

git_clone_repository \
    "$axmol_repository" v"$axmol_version" "$source_dir"

configure()
{
    cmake_options=()
    for definition in "${axmol_definitions[@]}"
    do
        cmake_options+=("-D$definition")
    done

    cd "$build_dir"
    cmake "$script_dir"/axmol \
          -DCMAKE_BUILD_TYPE="${build_type^}" \
          -DCMAKE_INSTALL_PREFIX="$install_dir" \
          -DCMAKE_PREFIX_PATH="$bomb_app_prefix" \
          -Daxmol_root="$source_dir" \
          "${cmake_options[@]}"
}

clean_up_install()
{
    local axmol_include="${install_dir:?}"/include/axmol

    find "$axmol_include" \
         \( \
         -name "*-android.*" \
         -o -name "*-apple.*" \
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
       "${axmol_include:?}"/platform/android \
       "${axmol_include:?}"/platform/apple \
       "${axmol_include:?}"/platform/ios \
       "${axmol_include:?}"/platform/mac \
       "${axmol_include:?}"/platform/win32 \
       "${axmol_include:?}"/platform/winrt
}

install_cmake_file()
{
    local cmake_module_dir="$install_dir"/lib/cmake/axmol/

    mkdir --parents "$cmake_module_dir"

    cat > "$cmake_module_dir"/axmol-config.cmake <<EOF
if(TARGET axmol::axmol)
  return()
endif()

find_path(
  axmol_include_dir
  NAMES axmol.h
  PATH_SUFFIXES axmol
)

set(
  axmol_definitions
$(printf "  %s\n" "${axmol_definitions[@]}")
)

function(link_axmol_library name)
  unset(axmol_dependency CACHE)
  find_library(axmol_dependency NAMES "\${name}" REQUIRED)
  set(axmol_libraries "\${axmol_libraries}" "\${axmol_dependency}" PARENT_SCOPE)
endfunction()

$(printf "link_axmol_library(%s)\n" "${axmol_link_libraries[@]}")

add_library(axmol::axmol INTERFACE IMPORTED)

cmake_path(GET axmol_include_dir PARENT_PATH axmol_parent_include_dir)

set_target_properties(
  axmol::axmol
  PROPERTIES
  INTERFACE_COMPILE_DEFINITIONS "\${axmol_definitions}"
  INTERFACE_INCLUDE_DIRECTORIES
    "\${axmol_include_dir};\${axmol_parent_include_dir};\${axmol_parent_include_dir}/GLFW"
  INTERFACE_LINK_LIBRARIES "\${axmol_libraries}"
)

cmake_path(GET axmol_parent_include_dir PARENT_PATH axmol_system_root)

file(GLOB axmol_shader_files \${CMAKE_PREFIX_PATH}/share/axmol/shaders/*)
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

configure
time cmake --build . --target install --parallel

clean_up_install
install_cmake_file

package_and_install \
    "$install_dir" axmol "$version" "$build_type"
