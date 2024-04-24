#!/bin/bash

set -euo pipefail

if [[ $# -ne 1 ]] || [[ "$1" = "--help" ]] || [[ "$1" = "-h" ]]
then
    echo "Run the test programs from the given build directory."
    echo "Usage: $0 DIR"
    exit 1
fi

search_directory="$1"
pass_count=0
test_count=0

while read -r bin
do
    test_count=$((test_count + 1))

    echo "Running '$bin'"

    if "$bin" --gtest_brief=1
    then
        pass_count=$((pass_count + 1))
    fi
done < <(find "$search_directory" -name "*-tests" -type f -executable)

echo "Passes: $pass_count/$test_count"

((pass_count == test_count))
