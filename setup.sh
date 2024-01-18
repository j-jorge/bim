#!/bin/bash

# Disable info message about not following sourced scripts.
# shellcheck disable=SC1091

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
backroom="$script_dir/.backroom"
host_prefix="$backroom"/host
python_virtual_environment_path="$host_prefix"/python
build_types=(release)
target_platform=linux
all_build_types=(debug release asan tsan)

export bim_host_prefix="$host_prefix"
export PATH="$script_dir/setup/bin/:$PATH"

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
  --build-type T
     Build for this configuration (debug, release, asan, tsan).
  --help, -h
     Display this message and exit.
  --target-platform P
     Build for platform P (either linux or android).
EOF
}

while (("$#" != 0))
do
    arg="$1"
    shift

    case "$arg" in
        --build-type)
            if [[ $# -eq 0 ]]
            then
                echo "Missing value for --build-type." >&2
                exit 1
            fi

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
        --target-platform)
            if [[ $# -eq 0 ]]
            then
                echo "Missing value for --target-platform." >&2
                exit 1
            fi

            target_platform="$1"
            shift
            ;;
        *)
            echo "Unknown argument '$arg'." >&2
            exit 1
            ;;
    esac
done

missing_dependencies=0

check_host_dependency ccache || missing_dependencies=1
check_host_dependency cmake || missing_dependencies=1
check_host_dependency git || missing_dependencies=1
check_host_dependency ninja || missing_dependencies=1
check_host_dependency python3 || missing_dependencies=1

if [[ "$target_platform" = "android" ]]
then
    check_host_dependency gradle || missing_dependencies=1
fi

if ((missing_dependencies != 0))
then
    exit 1
fi

if [[ -f "$script_dir/.setup.conf" ]]
then
    . "$script_dir/.setup.conf"
fi

: "${shell_utils_commit=004b72c863d93ea958eef9178d7b12324283e2a1}"
: "${shell_utils_repository:=https://github.com/j-jorge/shell-utils}"
: "${paco_commit=8259765a6e3f3de004ca165854653a7581a37cbc}"
: "${paco_repository:=https://github.com/j-jorge/cpp-package-manager}"

set_up_host_prefix()
{
    # Shell Utils
    echo -e "\033[1;32mInstalling the shell utils scripts\033[0;0m"
    mkdir --parents "$host_prefix"

    bim-git-clone-repository "$shell_utils_repository" \
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
    echo -e "${green_bold:-}Installing the package manager${term_color:-}"
    bim-git-clone-repository "$paco_repository" \
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
    local python
    python="$(command -v python3)"
    echo -e "${green_bold:-}Installing Python virtual environment ($python) ${term_color:-}"
    "$python" -m venv "$python_virtual_environment_path"
}

install_dependencies()
(
    export backroom
    export bim_build_type="$1"
    export bim_package_install_prefix="$2"
    export bim_package_install_platform="$3"
    export bim_packages_root="$backroom"/packages
    export bim_host_prefix="$host_prefix"
    export bim_target_platform="$target_platform"

    grep --invert-match "^#" "$4" \
        | while read -r script
    do
        if [[ -z "${script:-}" ]]
        then
            continue
        fi

        script="$script_dir"/setup/dependencies/recipes/"$script"

        if [[ -f "$script" ]] && [[ -x "$script" ]]
        then
            "$script"
        else
            echo "Missing dependency script: '$script'." >&2
        fi
    done
)

launch_build()
{
    local bim_build_type="$1"
    bim_app_prefix="$backroom"/"$target_platform"-"$bim_build_type"

    mkdir --parents "$bim_app_prefix"

    export PATH="$host_prefix"/bin:"$PATH"

    . "$python_virtual_environment_path"/bin/activate

    # Host dependencies: install everything in $host_prefix. Those are
    # tools to be used by the build system (e.g. tooling).
    echo -e "${green_bold}Installing host dependencies${term_color}"
    install_dependencies release \
                         "$host_prefix" \
                         linux \
                         "$script_dir"/setup/dependencies/host-dependencies.txt

    # App dependencies: install everything in $bim_app_prefix. Those
    # are dependencies required by the app (e.g. libraries).
    echo -e "${green_bold}Installing app dependencies${term_color}"
    install_dependencies "$bim_build_type" \
                         "$bim_app_prefix" \
                         "$target_platform" \
                         "$script_dir"/setup/dependencies/app-dependencies.txt

    # Actual build
    echo -e "${green_bold}Building${term_color}"

    build_dir="$script_dir"/build/"$target_platform"/"$bim_build_type"

    if [[ ! -f "$build_dir"/build.ninja ]]
    then
        rm --force --recursive "$build_dir"
        mkdir --parents "$build_dir"

        cmake_options=()
        case "$bim_build_type" in
            asan)
                cmake_options=(-DCMAKE_BUILD_TYPE=RelWithDebInfo
                               -DBIM_ADDRESS_SANITIZER=ON)
                ;;
            debug)
                cmake_options=(-DCMAKE_BUILD_TYPE=Debug)
                ;;
            release)
                cmake_options=(-DCMAKE_BUILD_TYPE=Release)
                ;;
            tsan)
                cmake_options=(-DCMAKE_BUILD_TYPE=RelWithDebInfo
                               -DBIM_THREAD_SANITIZER=ON)
                ;;
        esac

        cd "$build_dir"
        cmake "$script_dir" -G Ninja \
              -DCMAKE_PREFIX_PATH="$bim_host_prefix" \
              -DCMAKE_FIND_ROOT_PATH="$bim_host_prefix" \
              -DCMAKE_UNITY_BUILD=ON \
              -DCMAKE_UNITY_BUILD_BATCH_SIZE=65535 \
              -DCMAKE_C_COMPILER_LAUNCHER=ccache \
              -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
              -DBIM_TARGET="$target_platform" \
              -DBIM_TARGET_PREFIX="$bim_app_prefix" \
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
