#!/bin/bash

set -euo pipefail

pass_count=0
file_count=0

function check_file()
{
    local project_dir="$1"
    local so="$2"

    echo "Searching project dir '$project_dir' in '$so'."

    if strings "$so" | grep "^$project_dir" | sed 's/^/  /'
    then
        echo "Project path found in '$so'."
    else
        pass_count=$((pass_count + 1))
    fi
}

if [[ $# -ne 1 ]] || (printf '%s\n' "$@" | grep --quiet '^\(-h\|--help\)$')
then
    cat <<EOF
Check that the libraries in the APK have no full path reference to the
project's directory.

Usage: $0 [ -h ] DIR

Where DIR is the build directory.

OPTIONS:
  --help, -h
     Display this message then exit.
EOF
    exit 0
fi

project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"/..; pwd)"
apk="$(find "$1" -name "app-release.apk")"
apk="$(realpath --canonicalize-existing "$apk")"

tmp_dir="$(mktemp --directory)"

function clean_up()
{
    rm -fr "$tmp_dir"
}
trap clean_up EXIT

cd "$tmp_dir"

unzip -x "$apk"

while read -r so
do
    file_count=$((file_count + 1))
    check_file "$project_dir" "$so"
done < <(find . -name "*.so")

echo "Passes: $pass_count/$file_count"

((pass_count == file_count))
