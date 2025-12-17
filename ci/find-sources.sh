#!/bin/bash

set -euo pipefail

script_dir="$(dirname "${BASH_SOURCE[0]}")"
repository_root="$(cd "$script_dir"/..; pwd)"

exclude_args=()

while read -r ignored
do
    if (( ${#exclude_args[@]} != 0 ))
    then
        exclude_args+=("-o")
    fi

    if [[ "${ignored::1}" = / ]]
    then
        ignored="${ignored:1}"
    fi


    if [[ "${ignored: -1}" = / ]]
    then
        exclude_args+=("-path" "$repository_root"/"${ignored::-1}"
                       "-o" "-path" "$repository_root/$ignored*"
                      )
    else
        exclude_args+=("-path" "$repository_root"/"$ignored")
    fi
done < <(
    grep --invert-match \
         --no-filename \
         '^#' \
         "$repository_root"/.gitignore \
         "$repository_root"/.git/info/exclude
    echo .git
)

find "$repository_root" -not \( "${exclude_args[@]}" -prune \) \( "$@" \)
