#!/bin/bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
backroom="$script_dir/.backroom"
host_prefix="$backroom"/host-prefix
python_virtual_environment_path="$host_prefix"/python
build_types=(release)
all_build_types=(debug release asan tsan)

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
  --build-type T   Build for this configuration (debug, release, asan, tsan).
  --help, -h       Display this message and exit.
EOF
}

while (("$#" != 0))
do
    arg="$1"
    shift

    case "$arg" in
        --build-type)
            if [[ "$1" = "all" ]]
            then
                build_types=("${all_build_types[@]}")
            else
                build_types=("$1")
            fi
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

if [[ -f "$script_dir/.setup.conf" ]]
then
    . "$script_dir/.setup.conf"
fi

: "${shell_utils_commit=1a4cdb63b13f115264635ada9778c12e47838586}"
: "${shell_utils_repository:=https://github.com/j-jorge/shell-utils}"
: "${paco_commit=188e7ece603c8b0c275e7f82a5bee6fdab7108b9}"
: "${paco_repository:=https://github.com/j-jorge/cpp-package-manager}"

set_up_host_prefix()
{
    # Shell Utils
    echo -e "\033[1;32mInstalling the shell utils scripts\033[0;0m"
    mkdir --parents "$host_prefix"

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

    # Python
    local python="$(command -v python3)"
    echo -e "${green_bold}Installing Python virtual environment ($python) ${term_color}"
    "$python" -m venv "$python_virtual_environment_path"
}

install_dependencies()
(
    export backroom
    export bomb_app_prefix="$2"
    export bomb_build_type="$1"
    export bomb_packages_root="$backroom"/packages
    export -f git_clone_repository
    export -f git_fetch

    while read -r script
    do
        if [[ -z "${script:-}" ]]
        then
            continue
        fi

        if [[ -f "$script_dir"/dependencies/"$script" ]]
        then
            "$script_dir"/dependencies/"$script"
        else
            echo "Missing dependency script: '$script_dir/dependencies/$script'." \
                 >&2
        fi
    done < "$3"
)

launch_build()
{
    local bomb_build_type="$1"
    bomb_app_prefix="$backroom"/linux-prefix-"$bomb_build_type"

    mkdir --parents "$bomb_app_prefix"

    export PATH="$host_prefix"/bin:"$PATH"

    . "$python_virtual_environment_path"/bin/activate

    # App dependencies.
    echo -e "${green_bold}Installing host dependencies${term_color}"
    install_dependencies release \
                         "$host_prefix"\
                         "$script_dir"/dependencies/host-dependencies.txt

    echo -e "${green_bold}Installing app dependencies${term_color}"
    install_dependencies "$bomb_build_type" \
                         "$bomb_app_prefix" \
                         "$script_dir"/dependencies/app-dependencies.txt

    # Actual build
    echo -e "${green_bold}Building${term_color}"

    build_dir="$script_dir"/build/"$bomb_build_type"

    if [[ ! -f "$build_dir"/build.ninja ]]
    then
        rm --force --recursive "$build_dir"
        mkdir --parents "$build_dir"

        cmake_options=()
        case "$bomb_build_type" in
            asan)
                cmake_options=(-DCMAKE_BUILD_TYPE=RelWithDebInfo
                               -DBOMB_ADDRESS_SANITIZER=ON)
                ;;
            debug)
                cmake_options=(-DCMAKE_BUILD_TYPE=Debug)
                ;;
            release)
                cmake_options=(-DCMAKE_BUILD_TYPE=Release)
                ;;
            tsan)
                cmake_options=(-DCMAKE_BUILD_TYPE=RelWithDebInfo
                               -DBOMB_THREAD_SANITIZER=ON)
                ;;
        esac

        cd "$build_dir"
        cmake "$script_dir" -G Ninja \
              -DCMAKE_PREFIX_PATH="$bomb_app_prefix" \
              -DCMAKE_UNITY_BUILD=ON \
              -DCMAKE_UNITY_BUILD_BATCH_SIZE=65535 \
              -DCMAKE_C_COMPILER_LAUNCHER=ccache \
              -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
              "${cmake_options[@]}"
        cd - > /dev/null
    fi

    cd "$build_dir"
    cmake --build . --parallel
}

set_up_host_prefix

for build_type in "${build_types[@]}"
do
    echo -e "\033[1;35m== Build type '$build_type' ==\033[0;0m"
    launch_build "$build_type"
done
