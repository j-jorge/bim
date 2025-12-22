#!/bin/bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
languages=()

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
  --language LANGUAGE_CODE…
     List of languages for which to generate the captures.
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
        --language)
            if (( $# == 0 ))
            then
                echo "Missing value for --language" >&2
                exit 1
            fi

            while (( $# != 0 )) && [[ "$1" != --* ]]
            do
                languages+=("$1")
                shift
            done
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

if [[ "${#languages[@]}" -eq 0 ]]
then
    languages=(br de en es fr it kab oc pt pt_BR tr)
fi

for language in "${languages[@]}"
do
    run_script "$language"
done

for s in "${failing[@]}"
do
    echo "FAIL: $s"
done

fail_count="${#failing[@]}"
echo "Passes: $((run_count-fail_count))/$run_count"

(( fail_count == 0 ))
