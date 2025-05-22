#!/bin/bash

set -euo pipefail

grep -h 'Trying to add session [0-9]\+ in existing encounter' "$@" \
    | sed 's/^.\([0-9]\+-[0-9]\+-[0-9]\+\) \([0-9]\+\)\+.\+ session \([0-9]\+\).\+/\1-\2 \3/' \
    | uniq \
    | cut -d' ' -f1 \
    | uniq -c \
    | awk '{print $2, $1}'
