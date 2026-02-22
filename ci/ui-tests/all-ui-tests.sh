#!/bin/bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

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

failing=()
run_count=0

while read -r test_file
do
    run_count=$((run_count + 1))

    if ! "$test_file" "$@"
    then
        failing+=("$test_file")
    fi
done < <(find "$script_dir" \
              -name "*.sh" \
              -executable \
              -exec grep --files-with-matches '^# *UI_TEST$' '{}' ';')

printf -- '-%.0s' {1..80}
echo

for s in "${failing[@]}"
do
    echo "FAIL: $s"
done

fail_count="${#failing[@]}"
echo "Pass: $((run_count-fail_count))/$run_count"

(( fail_count == 0 ))
