#!/bin/bash

set -eu

function usage()
{
    cat <<EOF
Test the crash handler of the server.

Usage: $0 [ OPTIONS ] DIR

Where DIR is the build directory.

OPTIONS:
  --help, -h
     Display this message then exit.
EOF
}

while [[ $# -ne 0 ]]
do
    arg="$1"
    shift
    case "$arg" in
        --help|-h)
            show_help=1
            ;;
        *)
            build_dir="$arg"
            ;;
    esac
done

if [[ "${show_help:-0}" = 1 ]]
then
    usage
    exit 0
fi

bim_server="$build_dir"/apps/server/bim-server

test_count=0
pass_count=0

test_crash()
{
    test_count=$((test_count + 1))

    if "$bim_server" --testing-crash 2>&1 \
            | grep --quiet \
                   '^#[0-9].\+ in crash_handler\((int)\)\? at .\+/apps/server/server-main.cpp:'
    then
        echo "test_crash: OK"
        pass_count=$((pass_count + 1))
    else
        "$bim_server" --testing-crash 2>&1 || true
        echo "test_crash: FAIL"
    fi
}

test_throw()
{
    test_count=$((test_count + 1))

    if "$bim_server" --testing-throw 2>&1 \
            | grep --quiet \
                   '^#[0-9].\+ in force_throw\(()\)\? at .\+/apps/server/server-main.cpp:'
    then
        echo "test_throw: OK"
        pass_count=$((pass_count + 1))
    else
        "$bim_server" --testing-throw 2>&1 || true
        echo "test_throw: FAIL"
    fi
}

test_crash
test_throw

if ((test_count == 0))
then
    echo "No test program to run."
else
    echo "Test programs: $pass_count/$test_count"
fi

((pass_count == test_count))
