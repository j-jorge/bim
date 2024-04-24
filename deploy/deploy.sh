#!/bin/bash

set -euo pipefail

function usage()
{
    cat <<EOF
Usage:
  $0 OPTIONS

Where OPTIONS is:
  --build-dir DIR
     Mandatory. The build directory from which bim-server will be copied.
  -h, --help
     Display this message and exit.
  --target LOGIN@HOST
     Mandatory. The destination onto which the server will be deployed.
EOF
}

if [[ $# -eq 0 ]]
then
    usage
    exit 1
fi

while [[ $# -ne 0 ]]
do
    arg="$1"
    shift

    case "$arg" in
        --build-dir)
            build_dir="${1:-}"
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        --target)
            login_at_host="${1:-}"
            shift
            ;;
    esac
done

if [[ -z "${build_dir:-}" ]]
then
    echo "Missing value for --build-dir. See --help for details." >&2
    exit 1
fi

if [[ -z "${login_at_host:-}" ]]
then
    echo "Missing value for --target. See --help for details." >&2
    exit 1
fi

ssh "$login_at_host" \
    mkdir --parents bim/{bin,etc,log} \
    '&&' cd bim/ \
    '&&' \( [[ ! -f docker-compose.yml ]] '||' docker-compose down \)

rsync "$build_dir"/apps/server/bim-server "$login_at_host":bim/bin/

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
rsync --recursive "$script_dir"/docker-compose.yml \
      "$script_dir"/etc \
      "$login_at_host":bim/

ssh "$login_at_host" \
    cd bim/  \
    '&&' docker-compose up --detach
