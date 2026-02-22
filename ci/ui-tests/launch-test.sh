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
Run one UI test of Bim!

${BASH_SOURCE[0]} [OPTIONS] SCRIPTS…

OPTIONS
  --build DIR
     Path to the build directory.
  --help, -h
     Display this message then exit.
  --use-valgrind
     Run the test programs with Valgrind. The test failures will be
     ignored, and the exit code will reflect the existence of errors
     found by Valgrind.
  --working-directory WORKING_DIRECTORY
     Enter this directory before running the test. See below.
  --
     Stop processing options, the rest of the command line is
     forwarded to Bim!.

The test runs by default in the DIR/ui-tests directory, where DIR is
the directory passed to --build. If --working-directory is provided
then the test will run in DIR/ui-tests/WORKING_DIRECTORY.

SCRIPTS… is the list of JSON scenarios to pass to the application.
EOF
}

if printf '%s\n' "$@" | grep --quiet '^\(-h\|--help\)$'
then
    usage
    exit 0
fi

test_directory=""
scripts=()

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
        --working-directory)
            if (( $# == 0 ))
            then
                echo "Missing value for --working-directory" >&2
                exit 1
            fi

            test_directory="$1"
            shift
            ;;
        --)
            break
            ;;
        *)
            scripts+=("$(readlink --canonicalize "$arg")")
            ;;
    esac
done

[[ -n "${build_dir:-}" ]] || fail "--build is not set."
[[ "${#scripts[@]}" -ne 0 ]] || fail "No script provided."

build_dir="$(readlink --canonicalize "$build_dir")"
working_directory="$build_dir"/ui-tests

rm --force --recursive "$working_directory"
mkdir --parents "$working_directory"
cd "$working_directory"

command=("$script_dir"/../run-ui-script.sh
         --build "$build_dir"
         --script "${scripts[@]}"
         --server 10000
         --montage montage.png)

if [[ -n "$test_directory" ]]
then
    mkdir --parents "$test_directory"
    command+=(--working-directory "$test_directory")
fi

if (( use_valgrind == 1 ))
then
    command+=(--use-valgrind)
fi

command+=(--
          --number-screenshots
          --scale 0.5
          "$@")

if "${command[@]}"
then
    echo PASS
else
    echo "${command[@]}"
    echo FAIL

    exit 1
fi
