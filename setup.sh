#!/bin/bash

# Disable info message about not following sourced scripts.
# shellcheck disable=SC1091

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
backroom="$script_dir/.backroom"
host_prefix="$backroom"/host
python_virtual_environment_path="$host_prefix"/python
build_type=release
incremental_build=0
target_platform=linux
tag=
build_steps=()
all_build_steps=(dependencies configure build test)

export bim_host_prefix="$host_prefix"
export PATH="$script_dir/setup/bin/:$PATH"

custom_cflags="-fvisibility=hidden"
custom_cflags+=" -fmacro-prefix-map=$script_dir/=./"
export CFLAGS="$custom_cflags ${CFLAGS:-}"
export CXXFLAGS="$custom_cflags ${CXXFLAGS:-}"

export CMAKE_GENERATOR="${CMAKE_GENERATOR:-Ninja}"

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
  --build-steps G…
     The build steps to execute (dependencies, configure, build,
     test). The order does not matter. By default, all steps are
     executed.
  --help, -h
     Display this message and exit.
  --incremental
     Do an incremental build, i.e. disable the unity build. This is
     useful if you need your build to take longer than necessary.
  --target-platform P
     Build for platform P (either linux or android).
  --tag T
     Use this tag to name the build folder, as in build/linux/T. The
     default is to use the build type as the tag.
EOF
}

build_step_is_enabled()
{
    (( ${#build_steps[@]} == 0 )) \
        || ( printf '%s\n' "${build_steps[@]}" | grep --quiet "^$1\$" )
}

while (("$#" != 0))
do
    arg="$1"
    shift

    case "$arg" in
        --build-steps)
            if (( $# == 0 ))
            then
                echo "Missing value for --build-steps." >&2
                exit 1
            fi

            while (( $# != 0 )) && [[ "$1" != --* ]]
            do
                if ! printf '%s\n' "${all_build_steps[@]}" \
                        | grep --quiet "^$1\$"
                then
                    echo "Unknown build step '$1'." >&2
                    exit 1
                fi

                build_steps+=("$1")
                shift
            done
            ;;
        --build-type)
            if (( $# == 0 ))
            then
                echo "Missing value for --build-type." >&2
                exit 1
            fi

            build_type="$1"
            shift
            ;;
        --help|-h)
            usage
            exit
            ;;
        --incremental)
            incremental_build=1
            ;;
        --tag)
            if (( $# == 0 ))
            then
                echo "Missing value for --tag." >&2
                exit 1
            fi

            tag="$1"
            shift
            ;;
        --target-platform)
            if (( $# == 0 ))
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
check_host_dependency make || missing_dependencies=1
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

if [[ -z "${tag:-}" ]]
then
    tag="${build_type}"
fi

if [[ -f "$script_dir/.setup.conf" ]]
then
    . "$script_dir/.setup.conf"
fi

: "${shell_utils_commit=bfaa4403e50ae92829c6054a2e9146fee6d8c57f}"
: "${shell_utils_repository:=https://github.com/j-jorge/shell-utils}"
: "${paco_commit=103721bb368722ead1815a84e9ebe3949f4fffc0}"
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

    export PATH="$host_prefix"/bin:"$PATH"

    # Package manager
    echo -e "${green_bold:-}Installing the package manager.${term_color:-}"
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
    echo -e "${green_bold:-}Installing Python virtual environment ($python).${term_color:-}"
    "$python" -m venv "$python_virtual_environment_path"

    . "$python_virtual_environment_path"/bin/activate
}

install_dependencies()
(
    export backroom
    export bim_build_type="$1"
    export bim_package_install_prefix="$2"
    export bim_package_install_platform="$3"
    export bim_packages_root="$backroom"/packages
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

install_all_dependencies()
{
    # Host dependencies: install everything in $host_prefix. Those are
    # tools to be used by the build system (e.g. tooling).
    echo -e "${green_bold}Installing host dependencies.${term_color}"
    install_dependencies release \
                         "$host_prefix" \
                         linux \
                         "$script_dir"/setup/dependencies/host-dependencies.txt

    # App dependencies: install everything in $bim_app_prefix. Those
    # are dependencies required by the app (e.g. libraries).
    echo -e "${green_bold}Installing app dependencies.${term_color}"
    install_dependencies "$build_type" \
                         "$bim_app_prefix" \
                         "$target_platform" \
                         "$script_dir"/setup/dependencies/app-dependencies.txt
}

configure()
{
    echo -e "${green_bold}Configuring '$build_type'.${term_color}"

    if [[ -f "$build_dir"/build.ninja ]]
    then
        return
    fi

    rm --force --recursive "$build_dir"
    mkdir --parents "$build_dir"

    cmake_options=()
    case "$build_type" in
        asan)
            cmake_options=(-DCMAKE_BUILD_TYPE=Debug
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

    if (( incremental_build == 0 ))
    then
        cmake_options+=(
          -DCMAKE_UNITY_BUILD=ON
          -DCMAKE_UNITY_BUILD_BATCH_SIZE=65535
        )
    fi

    cd "$build_dir"
    cmake "$script_dir" -G Ninja \
          -DCMAKE_PREFIX_PATH="$bim_host_prefix" \
          -DCMAKE_FIND_ROOT_PATH="$bim_app_prefix;$bim_host_prefix" \
          -DCMAKE_C_COMPILER_LAUNCHER=ccache \
          -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
          -DBIM_TARGET="$target_platform" \
          -DBIM_TARGET_PREFIX="$bim_app_prefix" \
          "${cmake_options[@]}"
    cd - > /dev/null
}

launch_build()
{
    echo -e "${green_bold}Building '$build_type'.${term_color}"
    cmake --build "$build_dir" --parallel
}

launch_tests()
{
    local result=0

    "$script_dir"/ci/run-test-programs.sh "$build_dir" \
        || result=$((result + 1))

    "$script_dir"/ci/no-metadata-in-png.sh "$build_dir" \
        || result=$((result + 1))

    if [[ "$target_platform" == android ]] \
           && [[ "$build_type" == release ]]
    then
        "$script_dir"/ci/no-path-in-apk.sh "$build_dir" \
            || result=$((result + 1))
    fi

    return "$result"
}

set_up_host_prefix

export bim_app_prefix="$backroom"/"$target_platform"-"$build_type"
mkdir --parents "$bim_app_prefix"

! build_step_is_enabled dependencies || install_all_dependencies

build_dir="$script_dir"/build/"$target_platform"/"$tag"

if (( incremental_build != 0 ))
then
    build_dir="$build_dir"-incremental
fi

! build_step_is_enabled configure || configure
! build_step_is_enabled build || launch_build

! build_step_is_enabled test || launch_tests
