#!/bin/bash

set -euo pipefail

script_dir="$(dirname "${BASH_SOURCE[0]}")"
repository_root="$(cd "$script_dir"/..; pwd)"
cd "$repository_root"

pass_count=0
test_count=0

tmp_file="$(mktemp)"
trap clean_up EXIT

clean_up()
{
    rm "$tmp_file"
}

find_sources()
{
    local exclude_args=()

    while read -r ignored
    do
        if (( ${#exclude_args[@]} == 0 ))
        then
            exclude_args=("-path" ."$ignored*")
        else
            exclude_args+=("-o" "-path" ."$ignored*")
        fi
    done < .gitignore

    find . -not \( "${exclude_args[@]}" \) \( "$@" \) -print0
}

run_test()
{
    local title="$1"
    local command="$2"
    shift 2

    local command_args=()

    while (( $# != 0 ))
    do
        local arg="$1"
        shift

        if [[ "$arg" = -- ]]
        then
            break
        else
            command_args+=("$arg")
        fi
    done

    local find_args=("$@")

    echo "== $title =="
    test_count=$((test_count + 1))

    if command -v "$command" > /dev/null
    then
        "$command" --version

        if find_sources "${find_args[@]}" \
                | xargs -0 "$command" "${command_args[@]}"
        then
            echo "OK"
            pass_count=$((pass_count + 1))
        else
            echo "FAIL"
        fi
    else
        echo "Could not find '$command'."
    fi

    echo
}

custom_command_test()
{
    local title="$1"
    shift

    echo "== $title =="
    test_count=$((test_count + 1))


    if "$@"
    then
        echo "OK"
        pass_count=$((pass_count + 1))
    else
        echo "FAIL"
    fi

    echo
}

run_test "Validating C++/Java/JSON formatting." \
         clang-format \
         --dry-run \
         --Werror \
         -- \
         -name "*.[cht]pp" \
         -o -name "*.java" \
         -o -name "*.json"

run_test "Validating Python files." \
         black \
         --config .black \
         --check \
         -- \
         -name "*.py"

run_test "Validating shell scripts." \
         shellcheck \
         -- \
         -name "*.sh"

run_test "Validating YAML files." \
         yamllint \
         --no-warnings \
         -- \
         -name "*.yml"

custom_command_test "Validating assets attributions." \
                    "$script_dir"/check-authors.sh

custom_command_test "Validating metadata." \
                    "$script_dir"/check-metadata.sh

echo "Passes: $pass_count/$test_count"

((pass_count == test_count))
