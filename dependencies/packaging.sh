#!/bin/bash

install_package()
{
    local name="$1"
    local version="$2"
    local flavor="$3"

    paco-install --disable-remote \
                 --name "$name" \
                 --version "$version" \
                 --prefix "$bomb_app_prefix" \
                 --flavor "$flavor" \
                 --platform linux
}

package_and_install()
{
    local install_dir="$1"
    local name="$2"
    local version="$3"
    local flavor="$4"

    shift 4

    local publish_args=("--disable-remote"
                        "--root" "$install_dir"
                        "--name" "$name"
                        "--version" "$version"
                        "--flavor" "$flavor"
                        "--platform" linux)

    local dependency_version

    for d in "$@"
    do
        dependency_version="$(paco-info \
                                  --name "$d" \
                                  --prefix "$bomb_app_prefix" \
                                  | grep '^Version:' \
                                  | cut -d: -f2 \
                                  || true)"

        if [[ -z "${dependency_version:-}" ]]
        then
            echo "Unknown dependency package: $d" >&2
            exit 1
        fi

        publish_args+=("--requires" "$d"="$dependency_version")
    done

    paco-publish "${publish_args[@]}"

    install_package "$name" "$version" "$flavor"
}
