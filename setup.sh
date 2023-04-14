#!/bin/bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
backroom="$script_dir/.backroom"
bomb_build_type=debug

git_fetch()
{
    if [[ "$(git rev-parse HEAD 2>/dev/null)" = "$1" ]]
    then
        return
    fi

    git fetch --quiet --depth 1 origin "$1"
    git reset --quiet --hard FETCH_HEAD
}

git_clone_repository()
{
    local repository="$1"
    local commit="$2"
    local dirname="$3"

    if [[ -d "$dirname" ]]
    then
        pushd "$dirname" > /dev/null

        local remote
        remote="$(git config --get remote.origin.url || true)"

        if [[ "$remote" = "$repository" ]]
        then
            git_fetch "$commit"
            return
        fi

        popd > /dev/null
        rm --force --recursive "$dirname"
    fi

    mkdir --parents "$dirname"
    pushd "$dirname" > /dev/null

    git init --quiet
    git remote add origin "$repository"
    git_fetch "$commit"

    popd > /dev/null
}

check_host_dependency()
{
    if command -v "$1" > /dev/null
    then
        return
    fi

    echo -e "\033[1;31mThe program '$1' is required. Aborting\033[0;0m"
    return 1
}

usage()
{
    cat <<EOF
Usage: build.sh OPTIONS

Where OPTIONS is
  --config FILE    Load this configuration file before doing anything else.
  --build-type T   Build for this configuration (debug or release).
  --help, -h       Display this message and exit.
EOF
}

while (("$#" != 0))
do
    arg="$1"
    shift

    case "$arg" in
        --build-type)
            bomb_build_type="$1"
            shift
            ;;
        --config)
            . "$1"
            shift
            ;;
        --help|-h)
            usage
            exit
            ;;
    esac
done

missing_dependencies=0

check_host_dependency cmake || missing_dependencies=1
check_host_dependency git || missing_dependencies=1
check_host_dependency ninja || missing_dependencies=1

if ((missing_dependencies != 0))
then
    exit 1
fi

: "${shell_utils_commit=7704dbdb07dc3c9480cc44b3d64ce9152a6afd58}"
: "${shell_utils_repository:=https://github.com/j-jorge/shell-utils}"
: "${paco_commit=0b92d834ceee4d23a02ce138475e5afcc4569756}"
: "${paco_repository:=https://github.com/j-jorge/cpp-package-manager}"

host_prefix="$backroom"/host-prefix-"$bomb_build_type"
bomb_app_prefix="$backroom"/linux-prefix-"$bomb_build_type"

# Shell Utils
echo -e "\033[1;32mInstalling the shell utils scripts\033[0;0m"
mkdir --parents "$bomb_app_prefix" "$host_prefix"

git_clone_repository "$shell_utils_repository" \
                     "$shell_utils_commit" \
                     "$backroom"/repositories/shell-utils

cd "$backroom"/repositories/shell-utils
mkdir --parents build
cd build
cmake ../build-scripts/cmake -DCMAKE_INSTALL_PREFIX="$host_prefix" \
      > ../../shell-utils.configure.out.txt
cmake --build . --parallel --target install \
      > ../../shell-utils.build.out.txt

. "$host_prefix"/share/iscoolentertainment/shell/colors.sh

# Package manager
echo -e "${green_bold}Installing the package manager${term_color}"
git_clone_repository "$paco_repository" \
                     "$paco_commit" \
                     "$backroom"/repositories/cpp-package-manager

cd "$backroom"/repositories/cpp-package-manager
mkdir --parents build
cd build
cmake ../build-scripts/cmake -DCMAKE_INSTALL_PREFIX="$host_prefix" \
      > ../../cpp-package-manager.configure.out.txt
cmake --build . --parallel --target install \
      > ../../cpp-package-manager.build.out.txt

# App dependencies.
echo -e "${green_bold}Installing app dependencies${term_color}"

export backroom
export bomb_app_prefix
export bomb_build_type
export bomb_packages_root="$backroom"/packages
export PATH="$host_prefix":"$PATH"
export -f git_clone_repository
export -f git_fetch

find "$script_dir"/dependencies -mindepth 1 -maxdepth 1 -type f -executable \
    | sort \
    | while read -r script
do
    "$script"
done

# Actual build
echo -e "${green_bold}Building${term_color}"

build_dir="$script_dir"/build/"$bomb_build_type"

if [[ ! -f "$build_dir"/build.ninja ]]
then
    rm --force --recursive "$build_dir"
    mkdir --parents "$build_dir"

    cd "$build_dir"
    cmake "$script_dir" -G Ninja \
          -DCMAKE_BUILD_TYPE="${bomb_build_type^}" \
          -DCMAKE_PREFIX_PATH="$bomb_app_prefix" \
          -DCMAKE_UNITY_BUILD=ON \
          -DCMAKE_UNITY_BUILD_BATCH_SIZE=65535 \
          -DCMAKE_C_COMPILER_LAUNCHER=ccache \
          -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
    cd - > /dev/null
fi

cd "$build_dir"
cmake --build . --parallel
