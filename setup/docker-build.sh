#!/bin/bash

set -euo pipefail

usage()
{
    cat <<EOF
Runs setup.sh in the build Docker image.

Usage: $0 OPTIONS

Where OPTIONS is
  --
     Stop parsing the options, pass the rest to setup.sh
  --help, -h
     Display this message and exit.
  --volume V…
     Additional mount points to pass to docker run.

Unknown options are forwarded to setup.sh.
EOF
}

if printf '%s\n' "$@" | grep --quiet '^\(--help\|-h\)$'
then
    usage
    exit
fi

docker_args=()
setup_args=()

while [[ $# -ne 0 ]]
do
    arg="$1"
    shift

    case "$arg" in
        --)
            setup_args+=("$@")
            break
            ;;
        --volume)
            while [[ $# -ne 0 ]] && [[ "$1" != --* ]]
            do
                docker_args+=(--volume "$1")
                shift
            done
            ;;
        *)
            setup_args+=("$arg")
            ;;
    esac
done

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
repo_root="$(cd "$script_dir"/.. ; pwd)"

cd "$script_dir"
mkdir --parents "$repo_root"/build/docker/{cache,backroom,output}

build_image="$(grep '^FROM ' "$repo_root"/deploy/Dockerfile \
                        | sed 's/^FROM *//')"
run_image="$(grep '^FROM ' dockerfile.build \
                      | sed 's/^FROM *//')"

if [[ "$build_image" != "$run_image" ]]
then
    echo "Different build ($build_image) and run ($run_image) images." >&2
    exit 1
fi

image_hash="$(cat dockerfile.build install-minimal-environment.sh \
                  | md5sum \
                  | awk '{print $1}')"

image_name=bim-build-"$image_hash"
image_id="$(docker images --quiet "$image_name")"

if [[ -z "$image_id" ]]
then
    docker build --tag "$image_name" --file dockerfile.build .
fi

docker run \
       --interactive \
       --tty \
       --user "$(id --user)":"$(id --group)" \
       --volume "$repo_root":/bim/ \
       --volume "$repo_root"/build/docker/backroom:/bim/.backroom \
       --volume "$repo_root"/build/docker/cache:/home/ubuntu/.cache \
       --volume "$repo_root"/build/docker/output:/bim/build \
       "${docker_args[@]}" \
       "$image_name" \
       /bim/setup.sh "${setup_args[@]}"
