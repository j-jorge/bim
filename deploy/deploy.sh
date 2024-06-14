#!/bin/bash

set -euo pipefail

function usage()
{
    cat <<EOF
Deploy the server application (bim-server) to the given server and
with the given user.

The port on which the server listens must be passed as an argument to
this script. The deployment is done in a directory named with this
port, thus allowing multiple servers to run on the same host with
different ports. The idea being that it would make the transition
smoother for the clients: once a server is deployed all new
connections can be redirected to it, then the old one can be
deactivated when there is no active sessions anymore.

Usage:
  $0 OPTIONS

Where OPTIONS is:
  --build-dir DIR
     Mandatory. The build directory from which bim-server will be copied.
  -h, --help
     Display this message and exit.
  --port PORT
     Mandatory. The port on which the server will listen.
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
        --port)
            port="${1:-}"
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

if [[ -z "${port:-}" ]]
then
    echo "Missing value for --port. See --help for details." >&2
    exit 1
fi

ssh "$login_at_host" \
    mkdir --parents bim/"$port"/{bin,etc,log} \
    '&&' cd bim/"$port"/ \
    '&&' \( \
    [[ ! -f docker-compose.yml ]] '||' PORT="$port" docker-compose down \
    \)

rsync "$build_dir"/apps/server/bim-server "$login_at_host":bim/"$port"/bin/

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
rsync --recursive "$script_dir"/docker-compose.yml \
      "$script_dir"/etc \
      "$login_at_host":bim/"$port"/

ssh "$login_at_host" \
    cd bim/"$port"/  \
    '&&' PORT="$port" docker-compose up --detach
