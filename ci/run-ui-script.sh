#!/bin/bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
repo_root="$(cd "$script_dir"/../; pwd)"
pids=()

build_dir=
use_valgrind=0
montage=
port=
scripts=()
user_working_directory=

function fail()
{
    echo "$@" >&2
    exit 1
}

function usage()
{
    cat <<EOF
Run Bim! client with the given scripts.

OPTIONS
  --build DIR
     Path to the build directory.
  --help, -h
     Display this message then exit.
  --montage FILE
     Create a montage with all the captures from the working directory.
  --script FILE…
     The scripts to launch. Run one client per script.
  --server PORT
     Run a game server on the given port before launching the scripts.
  --use-valgrind
     Run the test programs with Valgrind. The test failures will be
     ignored, and the exit code will reflect the existence of errors
     found by Valgrind.
  --working-directory DIR
     The directory from where to run the clients.
EOF
}

if printf '%s\n' "$@" | grep --quiet '^\(-h\|--help\)$'
then
    usage
    exit 0
fi

while (( $# != 0 ))
do
    arg="$1"
    shift

    case "$arg" in
        --build)
            if (( $# == 0 ))
            then
                echo "Missing value for --build" >&2
                exit 1
            fi

            build_dir="$1"
            shift
            ;;
        --montage)
            if (( $# == 0 ))
            then
                echo "Missing value for --montage" >&2
                exit 1
            fi

            montage="$1"
            shift
            ;;
        --script)
            if (( $# == 0 ))
            then
                echo "Missing value for --script" >&2
                exit 1
            fi

            while (( $# != 0 )) && [[ "$1" != --* ]]
            do
                scripts+=("$(readlink --canonicalize "$1")")
                shift
            done
            ;;
        --server)
            if (( $# == 0 ))
            then
                echo "Missing value for --server" >&2
                exit 1
            fi

            port="$1"
            shift
            ;;
        --use-valgrind)
            use_valgrind=1
            ;;
        --working-directory)
            if (( $# == 0 ))
            then
                echo "Missing value for --working-directory" >&2
                exit 1
            fi

            user_working_directory="$(readlink --canonicalize "$1")"
            shift
            ;;
        --)
            break
            ;;
        *)
            echo "Unknown option $arg." >&2
            exit 1
            ;;
    esac
done

[[ -n "${build_dir:-}" ]] || fail "--build is not set."
(( "${#scripts[@]}" != 0 )) || fail "--scripts is not set."

build_dir="$(readlink --canonicalize "$build_dir")"
bim="$build_dir"/apps/linux/bim
server="$build_dir"/apps/server/bim-server

command=()

(( use_valgrind == 0 )) || command+=(valgrind
                                     "--quiet"
                                     "--error-exitcode=1"
                                     "--track-origins=yes"
                                     "--exit-on-first-error=yes"
                                     --)

command+=("$bim"
          --assets
          "$repo_root"/static-assets/
          "$build_dir"/assets/generated/
          --console-log)

(( use_valgrind == 0 )) || command+=(--script-step-timeout 20)

command+=("$@")

if printf '%s\n' "$@" | grep --quiet '^--app-dir'
then
    add_app_dir=0
else
    add_app_dir=1
fi

tmp_dir="$(mktemp --directory)"
server_pid=

if [[ -n "$port" ]]
then
    "$server" --console-log \
              --name "Bim! World's Server" \
              --port "$port" \
              > "$tmp_dir"/server.out \
              2> "$tmp_dir"/server.err &

    server_pid="$!"
    echo "Server PID is $server_pid."

    export BIM_GAME_SERVER_HOST=localhost:"$port"
fi

initial_directory="$(pwd)"
working_directory=()

for script in "${scripts[@]}"
do
    if jq '.actions[] | select(.kind == "capture")' "$script" \
            | grep --quiet .
    then
        if [[ -n "$user_working_directory" ]]
        then
            working_directory+=("$user_working_directory")
        else
            d="$(basename "$script" .json)"
            working_directory+=("$d")
            mkdir --parents "$d"
        fi
    else
        working_directory+=("")
    fi
done

i=0
for script in "${scripts[@]}"
do
    [[ -z "${working_directory[$i]}" ]] \
        || cd "${working_directory[$i]}"

    app_dir="$tmp_dir"/"$i"
    mkdir --parents "$app_dir"

    client_command=("${command[@]}"
                    --script "$script")

    if (( add_app_dir == 1 ))
    then
        client_command+=(--app-dir "$app_dir")
    fi

    echo "Running" "${client_command[@]}"

    "${client_command[@]}" > "$tmp_dir"/client.$i.out \
                           2> "$tmp_dir"/client.$i.err &
    p="$!"
    echo "Client #$i PID is $p."
    pids+=("$p")
    process_name["$p"]="Client #$i"

    i=$((i+1))
    cd "$initial_directory"
done

exit_code=0

i=0
for p in "${pids[@]}"
do
    if ! wait "$p"
    then
        echo "${process_name["$p"]} has failed."
        cat "$tmp_dir"/client.$i.err
        exit_code=1
    fi
    i=$((i+1))
done

if [[ -n "${server_pid:-}" ]]
then
    kill -INT "$server_pid"

    if ! wait "$server_pid"
    then
        echo "Server has failed."
        exit_code=1
    fi
fi

if (( exit_code == 0 ))
then
    rm --force --recursive "$tmp_dir"
else
    echo "Logs are in $tmp_dir"
fi

if [[ -n "$montage" ]]
then
    for d in "${working_directory[@]}"
    do
        if [[ -n "$d" ]]
        then
            cd "$d"
            rm --force "$montage"
            for f in *.png
            do
                magick "$f" \
                       -gravity North \
                       -fill black \
                       -splice 0x18 \
                       -annotate +0+2 \
                       "$f" miff:-
            done \
                | montage -geometry +2+2 -tile 6x miff:- "$montage"
            cd -

            if [[ -n "$user_working_directory" ]]
            then
                break
            fi
        fi
    done
fi

exit "$exit_code"
