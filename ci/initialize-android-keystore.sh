#!/bin/bash

set -euo pipefail

usage()
{
    cat <<EOF
Fill the keystore.properties file for the Android build, and the
keystore file itself for release builds. The values are taken from
environment variables BIM_KEYSTORE_PASSWORD and BIM_RELEASE_KEYSTORE.

Usage: $0 OPTIONS

Where OPTIONS are:
  --build-type TYPE
      Tell if we are doing a "release" build or a "debug" build. For
      the latter we will use the default debug signing key.
  --help, -h
      Display this message and exit.
  --output-dir DIR
      Where to put the generated files.
EOF
}

while (( $# != 0 ))
do
    arg="$1"
    shift

    case "$arg" in
        --build-type)
            if [[ "$1" = debug ]] || [[ "$1" = release ]]
            then
                build_type="$1"
            else
                echo "Unknown build type '$1'." >&2
                exit 1
            fi
            shift
            ;;
        --help)
            usage
            exit 0
            ;;
        --output-dir)
            if (( "$#" == 0 ))
            then
                echo "Missing value for --output-dir." >&2
                exit 1
            fi
            output_dir="$1"
            shift
            ;;
        *)
            echo "Unknown argument '$arg'." >&2
            ;;
    esac
done

if [[ -z "${build_type:-}" ]]
then
    echo "Option --build-type is required. See --help for details." >&2
    exit 1
fi

if [[ -z "${output_dir:-}" ]]
then
    echo "Option --output-dir is required. See --help for details." >&2
    exit 1
fi

if [[ ! -d "$output_dir" ]]
then
    echo "The directory '$output_dir' does not exist." >&2
    exit 1
fi

if [[ "$build_type" = release ]] && [[ -n "${BIM_RELEASE_KEYSTORE:-}" ]]
then
    echo "$BIM_RELEASE_KEYSTORE" \
        | base64 --decode > "$output_dir"/release.keystore
    cat > "$output_dir"/keystore.properties <<EOF
storePassword=$BIM_KEYSTORE_PASSWORD
keyPassword=$BIM_KEYSTORE_PASSWORD
EOF
else
    cat > "$output_dir"/keystore.properties <<EOF
storePassword=
keyPassword=
EOF
fi
