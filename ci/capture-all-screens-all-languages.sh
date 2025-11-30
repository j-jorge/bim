#!/bin/bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

function fail()
{
    echo "$@" >&2
    exit 1
}

function usage()
{
    cat <<EOF
Generate the screen captures for the metadata folder.

OPTIONS
  --build DIR
     Path to the build directory.
  --help, -h
     Display this message then exit.
EOF
}

if printf '%s\n' "$@" | grep --quiet '^\(-h\|--help\)$'
then
    usage
    exit 0
fi

while (( $# != 0 ))
do
    arg="$1"
    shift

    case "$arg" in
        --build)
            if (( $# == 0 ))
            then
                echo "Missing value for --build" >&2
                exit 1
            fi

            build_dir="$1"
            shift
            ;;
    esac
done

[[ -n "${build_dir:-}" ]] || fail "--build is not set."

build_dir="$(readlink --canonicalize "$build_dir")"

failing=()
run_count=0

function run_script()
{
    run_count=$((run_count+1))

    local lang="$1"

    printf -- '-%.0s' {1..80}
    echo
    echo "LANG=$lang"
    printf -- '-%.0s' {1..80}
    echo

    local working_directory="$build_dir"/captures/"$lang"

    rm --force --recursive "$working_directory"
    mkdir --parents "$working_directory"

    local scripts=("$script_dir"/language-capture-scripts/main.json
                   "$script_dir"/language-capture-scripts/opponent.json)

    local command=("$script_dir"/run-ui-script.sh
                   --build "$build_dir"
                   --script "${scripts[@]}"
                   --server 10000
                   --working-directory "$working_directory"
                   --montage montage.png
                   --
                   --number-screenshots
                   --scale 0.5)

    printf -- '-%.0s' {1..80}
    echo
    echo "${command[@]}"

    LANG="$lang" "${command[@]}" \
        || failing+=("$lang")
}

run_script br
run_script de
run_script fr
run_script pt
run_script pt_BR
run_script tr

for s in "${failing[@]}"
do
    echo "FAIL: $s"
done

fail_count="${#failing[@]}"
echo "Passes: $((run_count-fail_count))/$run_count"

(( fail_count == 0 ))
