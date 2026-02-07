#!/bin/bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
languages=()

function fail()
{
    echo "$@" >&2
    exit 1
}

function usage()
{
    cat <<EOF
Generate the screen captures for the metadata folder.

OPTIONS
  --build DIR
     Path to the build directory.
  --help, -h
     Display this message then exit.
  --language LANGUAGE_CODE…
     List of languages for which to generate the captures.
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
        --language)
            if (( $# == 0 ))
            then
                echo "Missing value for --language" >&2
                exit 1
            fi

            while (( $# != 0 )) && [[ "$1" != --* ]]
            do
                languages+=("$1")
                shift
            done
            ;;
    esac
done

[[ -n "${build_dir:-}" ]] || fail "--build is not set."

build_dir="$(readlink --canonicalize "$build_dir")"
repo_root="$(readlink --canonicalize "$script_dir"/..)"
metadata_dir="$repo_root"/metadata

failing=()
run_count=0

config_dir="$(mktemp --directory)"
cp "$script_dir"/metadata-scripts/preferences.json "$config_dir"

function run_script()
{
    run_count=$((run_count+1))

    local scripts=()
    local lang="$1"
    local working_directory="$2"

    for s in  main opponent opponent opponent
    do
        scripts+=("$script_dir"/metadata-scripts/"$s".json)
    done

    local command=("$script_dir"/run-ui-script.sh
                 --build "$build_dir"
                 --script "${scripts[@]}"
                 --server 10000
                 --working-directory "$working_directory"
                 --
                 --app-dir "$config_dir")

    printf -- '-%.0s' {1..80}
    echo
    echo "${command[@]}"

    if LANG="$lang" "${command[@]}"
    then
        find . -maxdepth 1 -name "*.png" \
            | while read -r f
        do
            convert "$f" "$(basename "$f" .png).jpg"
            rm --force "$f"
        done
    else
        failing+=("$1")
    fi
}

function generate_captures()
{
    local d="$metadata_dir"/"$2"/images/phoneScreenshots
    mkdir --parents "$d"

    printf '=%.0s' {1..80}
    echo
    echo "LANG=$1"
    printf '=%.0s' {1..80}
    echo
    run_script "$1" "$d"

    if [[ "$1" != en ]]
    then
        local en_screenshots_dir="$metadata_dir"/en-US/images/phoneScreenshots

        cp "$en_screenshots_dir"/screenshot-1-gameplay.jpg \
           "$en_screenshots_dir"/screenshot-5-gameplay.jpg \
           "$d"/
    fi
}

function generate_captures_for_directory()
{
    if [[ "$1" = "pt-BR" ]]
    then
        generate_captures pt_BR pt-BR
    else
        generate_captures "${1/-*/}" "$1"
    fi
}

all_directories=(br-FR
                 de-DE
                 en-US
                 es-ES
                 fr-FR
                 it-IT
                 kab-DZ
                 ko-KR
                 oc-FR
                 pt-PT
                 pt-BR
                 tr-TR
                 uk-UA
                )

if [[ "${#languages[@]}" -eq 0 ]]
then
    for d in "${all_directories[@]}"
    do
        generate_captures_for_directory "$d"
    done
else
    for d in "${all_directories[@]}"
    do
        language_code="${d/-*/}"
        if printf '%s' "${languages[@]}" \
                | grep --quiet "^\($language_code\|$d\)\$"
        then
            generate_captures_for_directory "$d"
        fi
    done
fi

for s in "${failing[@]}"
do
    echo "FAIL: $s"
done

fail_count="${#failing[@]}"
echo "Passes: $((run_count-fail_count))/$run_count"

rm --force --recursive "$config_dir"

(( fail_count == 0 ))
