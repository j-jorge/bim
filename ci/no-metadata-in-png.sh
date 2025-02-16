#!/bin/bash

set -euo pipefail

pass_count=0
file_count=0

function check_file()
{
    local png="$1"

    if identify -verbose "$png" \
            | grep --only-matching 'png:[a-zA-Z]\{4\}' \
            | grep --quiet --invert-match \
                   'png:\(IHDR\|PLTE\|IDAT\|IEND\|tRNS\)'
    then
        echo "Unwanted chunks found in '$png':"

        identify -verbose "$png" \
            | grep --only-matching 'png:[a-zA-Z]\{4\}' \
            | grep --invert-match 'png:\(IHDR\|PLTE\|IDAT\|IEND\|tRNS\)'
    else
        pass_count=$((pass_count + 1))
    fi
}

if printf '%s\n' "$@" | grep --quiet '^\(-h\|--help\)$'
then
    cat <<EOF
Check that PNG files have no chunks preventing reproducible builds
such as dates.

Usage: $0 [ -h ] DIR…

Where DIR… is the list of folders to scan for PNG files.

OPTIONS:
  --help, -h
     Display this message then exit.
EOF
    exit 0
fi

while read -r png
do
    file_count=$((file_count + 1))
    check_file "$png"
done < <(find "$@" -name "*.png" -not -wholename '*/intermediates/*')

echo "Passes: $pass_count/$file_count"

((pass_count == file_count))
