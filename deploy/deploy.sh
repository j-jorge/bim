#!/bin/bash

set -euo pipefail

function clean_up()
{
    if [[ -n "${temp_server_config:-}" ]]
    then
        rm --force "$temp_server_config"
    fi

    if [[ -n "${set_up_script:-}" ]]
    then
        rm --force "$set_up_script"
    fi
}

function usage()
{
    cat <<EOF
Deploy the server application (bim-server) to the given server and
with the given user.

The deployment is done in a directory named with the server's
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
  --config FILE
     The config file from which we get the configuration of this
     script for the app.
  -h, --help
     Display this message and exit.
EOF
}

if [[ $# -eq 0 ]]
then
    usage
    exit 1
fi

if printf '%s\n' "$@" | grep --quiet '^\(-h\|--help\)$'
then
    usage
    exit 0
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
        --config)
            if [[ "$#" -eq 0 ]]
            then
                echo "Missing value for --config." >&2
                exit 1
            fi
            config_file="$1"
            ;;
    esac
done

if [[ -z "${build_dir:-}" ]]
then
    echo "--build-dir is required." >&2
    exit 1
fi

if [[ -z "${config_file:-}" ]]
then
    echo "--config is required." >&2
    exit 1
fi

# shellcheck disable=SC1090
. "$config_file"

if [[ -z "${bim_etc:-}" ]]
then
    echo "bim_etc must be set." >&2
    exit 1
fi

if [[ -z "${bim_host:-}" ]]
then
    echo "bim_host must be set." >&2
    exit 1
fi

if [[ -z "${bim_port:-}" ]]
then
    echo "bim_port must be set." >&2
    exit 1
fi

if [[ -z "${bim_server_config:-}" ]]
then
    echo "bim_server_config:- must be set." >&2
    exit 1
fi

bim_prod_or_dev="${bim_prod_or_dev:-prod}"

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

echo "Replacing server listening on port $bim_port in 5 seconds."
sleep 5

echo "GO!"

if [[ -z "${build_dir:-}" ]]
then
    echo "Missing value for --build-dir. See --help for details." >&2
    exit 1
fi

temp_server_config="$(mktemp)"
jq --compact-output --slurp '.[0] * .[1]' \
   "$script_dir"/server-config.json \
   "$bim_server_config" \
   > "$temp_server_config"

set_up_script="$(mktemp)"

cat > "$set_up_script" <<EOF
#!/bin/bash

set -euo pipefail

if [[ -e bim/"$bim_port"/lock ]]
then
    echo "'bim/$bim_port/lock' exists. Aborting."
    exit 1
fi

mkdir --parents bim/"$bim_port"/{bin,etc/bim} \
      bim/"$bim_port"/persistent/{log,contest}

cd bim/"$bim_port"/
[[ ! -f docker-compose.yml ]] || PORT="$bim_port" docker-compose down
EOF

rsync "$set_up_script" "$bim_host":/tmp/bim-set-up.sh

ssh "$bim_host" \
    chmod u+x /tmp/bim-set-up.sh \
    '&&' /tmp/bim-set-up.sh \
    '&&' rm --force /tmp/bim-set-up.sh

bin_files=("$build_dir"/apps/server/bim-server
           "$build_dir"/apps/server/bim-stack-dump)

[[ ! -f "$build_dir"/apps/server/bim-server.dbg ]] \
    || bin_files+=("$build_dir"/apps/server/bim-server.dbg)

rsync "${bin_files[@]}" \
      "$bim_host":bim/"$bim_port"/bin/

rsync --recursive \
      "$script_dir"/bin \
      "$script_dir"/docker-compose.yml \
      "$script_dir"/Dockerfile \
      "$script_dir"/etc \
      "$bim_etc" \
      "$bim_host":bim/"$bim_port"/

rsync "$temp_server_config" \
      "$bim_host":bim/"$bim_port"/etc/bim/server-config.json

ssh "$bim_host" \
    cd bim/"$bim_port"/  \
    '&&' PORT="$bim_port" docker-compose up --build --detach

if [[ "$bim_prod_or_dev" = prod ]]
then
    ssh "$bim_host" touch bim/"$bim_port"/lock
fi
