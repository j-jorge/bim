#!/bin/bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
use_valgrind=0

function fail()
{
    echo "$@" >&2
    exit 1
}

function usage()
{
    cat <<EOF
Run all UI tests of Bim!

OPTIONS
  --build DIR
     Path to the build directory.
  --help, -h
     Display this message then exit.
  --use-valgrind
     Run the test programs with Valgrind. The test failures will be
     ignored, and the exit code will reflect the existence of errors
     found by Valgrind.
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
        --use-valgrind)
            use_valgrind=1
            ;;
        *)
            echo "Unknown option $arg." >&2
            exit 1
            ;;
    esac
done

[[ -n "${build_dir:-}" ]] || fail "--build is not set."

build_dir="$(readlink --canonicalize "$build_dir")"
working_directory="$build_dir"/ui-tests

rm --force --recursive "$working_directory"
mkdir --parents "$working_directory"
cd "$working_directory"

failing=()
run_count=0

function run_script()
{
    run_count=$((run_count+1))

    local scripts=()
    local working_directory=""

    if (( $# > 1 ))
    then
        working_directory="$1"
        mkdir --parent "$working_directory"
        shift
    fi

    for s in "$@"
    do
        scripts+=("$script_dir"/capture-scripts/"$s")
    done

    local command=("$script_dir"/run-ui-script.sh
                 --build "$build_dir"
                 --script "${scripts[@]}"
                 --server 10000
                 --montage montage.png)

    if [[ -n "$working_directory" ]]
    then
        command+=(--working-directory "$working_directory")
    fi

    if (( use_valgrind == 1 ))
    then
        command+=(--use-valgrind)
    fi

    command+=(--
              --number-screenshots
              --scale 0.5)

    printf -- '-%.0s' {1..80}
    echo
    echo "${command[@]}"
    "${command[@]}" || failing+=("$1")
}

run_script game-features-buttons.json
run_script lobby-buttons.json
LANG=en run_script language-buttons-in-english.json
LANG=fr run_script english-language-button.json
run_script settings-buttons.json
run_script shop-buttons.json
run_script 2-players \
           2-players-game-player-1-wins.json \
           2-players-game-player-2-loses.json
run_script 3-players \
           3-players-game-player-1-wins.json \
           3-players-game-player-2-loses.json \
           3-players-game-player-3-loses.json
run_script 4-players \
           4-players-game-player-1-wins.json \
           4-players-game-player-2-loses.json \
           4-players-game-player-3-loses.json \
           4-players-game-player-4-loses.json
run_script 2-players-draw \
            2-players-game-player-1-loses.json \
            2-players-game-player-2-loses.json

for s in "${failing[@]}"
do
    echo "FAIL: $s"
done

fail_count="${#failing[@]}"
echo "Passes: $((run_count-fail_count))/$run_count"

(( fail_count == 0 ))
