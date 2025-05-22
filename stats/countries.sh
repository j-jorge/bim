#!/bin/bash

set -euo pipefail

grep -h -o 'ip=.\+' "$@" \
    | cut -d= -f2 \
    | cut -d. -f1-4 \
    | xargs -n 1 geoiplookup \
    | sort \
    | uniq -c \
    | sort -g
