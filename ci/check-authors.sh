#!/bin/bash

set -euo pipefail

authors_file=AUTHORS.md
repository_root="$(cd "$(dirname "${BASH_SOURCE[0]}")"/..; pwd)"
cd "$repository_root"

function list_assets_in_repository()
{
    find assets static-assets \
         -name "*.png" \
         -o -name "*.jpg" \
         -o -name "*.ogg" \
         -o -name "*.ttf"
}

function list_assets_from_authors_file()
{
    grep '^\(assets\|static-assets\)/' "$authors_file"
}

readarray -t repository_assets < <(list_assets_in_repository)
missing=0

for a in "${repository_assets[@]}"
do
    if ! grep --quiet --perl-regexp "^\Q$a\E$" "$authors_file"
    then
        missing=$((missing + 1))
        echo "Missing author for '$a'."
    fi
done

readarray -t listed_assets < <(list_assets_from_authors_file)
extra=0

for a in "${listed_assets[@]}"
do
    if [[ ! -f "$a" ]]
    then
        extra=$((extra + 1))
        echo "Asset does not exist '$a'."
    fi
done

exit_code=0

if (( missing != 0 ))
then
    echo "Error: $missing assets without attribution."
    exit_code=1
fi

if (( extra != 0 ))
then
    echo "Error: $extra extra assets listed in $authors_file."
    exit_code=1
fi

exit $exit_code
