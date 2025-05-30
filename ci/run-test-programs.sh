#!/bin/bash

set -euo pipefail

function usage()
{
    cat <<EOF
Run the test programs from the given build directory.

Usage: $0 [ OPTIONS ] DIR…

Where DIR… is the list of folders to scan for test programs.

OPTIONS:
  --help, -h
     Display this message then exit.
  --use-valgrind
     Run the test programs with Valgrind. The test failures will be
     ignored, and the exit code will reflect the existence of errors
     found by Valgrind.
  --verbose
     Display the name of each executed test.
EOF
}

search_directories=()

while [[ $# -ne 0 ]]
do
    arg="$1"
    shift
    case "$arg" in
        --help|-h)
            show_help=1
            ;;
        --use-valgrind)
            use_valgrind=1
            ;;
        --verbose)
            verbose=1
            ;;
        *)
            search_directories+=("$arg")
            ;;
    esac
done

if [[ "${show_help:-0}" = 1 ]]
then
    usage
    exit 0
fi

if [[ "${#search_directories[@]}" -eq 0 ]]
then
    echo "Missing directories to scan. Try --help for help." >&2
    exit 1
fi

pass_count=0
test_count=0

if [[ "${use_valgrind:-0}" = 1 ]]
then
    test_args=("--ignore-failures")
    launcher=(valgrind
              "--quiet"
              "--error-exitcode=1"
              "--track-origins=yes"
              "--exit-on-first-error=yes")
else
    test_args=()
    launcher=()
fi

if [[ "${verbose:-0}" = 0 ]]
then
    test_args+=("--gtest_brief=1")
fi

while read -r bin
do
    test_count=$((test_count + 1))

    echo "Running '$bin'"

    if "${launcher[@]}" "$bin" "${test_args[@]}"
    then
        pass_count=$((pass_count + 1))
    fi
done < <(find "${search_directories[@]}" -name "*-tests" -type f -executable)

if ((test_count == 0))
then
    echo "No test program to run."
else
    echo "Test programs: $pass_count/$test_count"
fi

((pass_count == test_count))
