#!/bin/bash

set -euo pipefail

script_dir="$(dirname "${BASH_SOURCE[0]}")"
repository_root="$(cd "$script_dir"/..; pwd)"
cd "$repository_root"

pass_count=0
test_count=0

function check_files()
{
    local max_characters="$1"

    while read -r file_path
    do
        file_size="$(wc -c "$file_path" | awk '{print $1}')"
        test_count=$((test_count + 1))

        if ((file_size > max_characters))
        then
            echo "File $file_path is too large. $file_size > $max_characters."
        else
            pass_count=$((pass_count + 1))
        fi
    done
}

check_files 80 \
            < <(find "$repository_root"/metadata -name "short_description.txt")
check_files 4000 \
            < <(find "$repository_root"/metadata -name "full_description.txt")
check_files 500 \
            < <(find "$repository_root"/metadata/ \
                     -wholename "*/changelogs/*.txt")

echo "Metadata validation: $pass_count/$test_count"

((pass_count == test_count))
