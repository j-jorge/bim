#!/bin/bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
repo_root="$(cd "$script_dir"/../; pwd)"
pids=()
exit_code=0

db_container_name="ui-test-business"
game_server_port=10000
business_server_db_port=5432
business_server_app_port=4210
# Export required for CLI tools from the business repo.
export BIM_BUSINESS_SERVER_URL="http://localhost:$business_server_app_port"
export BIM_BUSINESS_CLI_SILENT=1

bim_build_dir=
business_build_dir=
business_setup=
use_gdb=0
use_valgrind=0
montage=
scripts=()
user_working_directory=

function fail()
{
    echo "$@" >&2
    exit 1
}

function clean_up()
{
    if (( $? != 0 ))
    then
        exit_code=$((exit_code + 1))
    fi

    if [[ -n "${business_server_pid:-}" ]]
    then
        echo "Killing business server, pid='$business_server_pid'."

        kill -15 "$business_server_pid" 2>/dev/null || true

        # Check that the server is not running. If it still is then we
        # kill it.
        ! kill -0 "$business_server_pid" || kill -9 "$business_server_pid"
    fi

    echo "Stopping container '$db_container_name'."
    docker stop "$db_container_name" || true

    if [[ -n "${game_server_pid:-}" ]]
    then
        kill -INT "$game_server_pid"

        if ! wait "$game_server_pid"
        then
            echo "Server has failed."
            exit_code=$((exit_code + 1))
        fi
    fi

    if [[ -d "${tmp_dir:-}" ]]
    then
        if (( exit_code == 0 ))
        then
            rm --force --recursive "$tmp_dir"
        else
            echo "Logs are in $tmp_dir"
        fi
    fi

    return "$exit_code"
}

trap clean_up EXIT

function usage()
{
    cat <<EOF
Run Bim! client with the given scripts.

OPTIONS
  --bim-build-dir DIR
     Path to the build directory of Bim!.
  --business-build-dir DIR
     Path to the build directory of the business server.
  --business-setup SCRIPT
     Path to a script to launch to set the initial state of the
     business server.
  --help, -h
     Display this message then exit.
  --montage FILE
     Create a montage with all the captures from the working directory.
  --script FILE…
     The scripts to launch. Run one client per script.
  --use-gdb
     Run the test programs with GDB.
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
        --bim-build-dir)
            if (( $# == 0 ))
            then
                echo "Missing value for --bim-build-dir" >&2
                exit 1
            fi

            bim_build_dir="$1"
            shift
            ;;
        --business-build-dir)
            if (( $# == 0 ))
            then
                echo "Missing value for --business-build-dir" >&2
                exit 1
            fi

            business_build_dir="$1"
            shift
            ;;
        --business-setup)
            if (( $# == 0 ))
            then
                echo "Missing value for --business-setup" >&2
                exit 1
            fi

            business_setup="$1"
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
        --use-gdb)
            use_gdb=1
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

[[ -n "${bim_build_dir:-}" ]] || fail "--bim-build-dir is not set."
[[ -n "${business_build_dir:-}" ]] || fail "--business-build-dir is not set."
(( "${#scripts[@]}" != 0 )) || fail "--scripts is not set."

bim_build_dir="$(readlink --canonicalize "$bim_build_dir")"
bim="$bim_build_dir"/apps/linux/bim
game_server="$bim_build_dir"/apps/server/bim-server
business_build_dir="$(readlink --canonicalize "$business_build_dir")"
business_server="$business_build_dir"/bim-business
business_repo="$(cd "$business_build_dir"/../../; pwd)"

command=()

(( use_valgrind == 0 )) || command+=(valgrind
                                     "--quiet"
                                     "--error-exitcode=1"
                                     "--track-origins=yes"
                                     "--exit-on-first-error=yes"
                                     --)
(( use_gdb == 0 )) || command+=(gdb "--args")

command+=("$bim"
          --assets
          "$repo_root"/static-assets/
          "$bim_build_dir"/assets/generated/
          --console-log)

(( use_valgrind == 0 )) || command+=(--script-step-timeout 20)

command+=("$@")

tmp_dir="$(mktemp --directory)"
mkdir --parents "$tmp_dir"/business

# Start the business server.
"$business_repo"/tests/start-server.sh \
                "$business_server" \
                "$db_container_name" \
                ui-test-user \
                ui-test-password \
                ui-test-db \
                "$business_server_db_port" \
                "$business_server_app_port" \
                "$tmp_dir"/business \
    | tee "$tmp_dir"/start-server.txt

business_server_pid="$( (grep --max-count 1 '^PID=[0-9]\+$' \
                              "$tmp_dir"/start-server.txt \
                              | cut -d= -f2) \
                        || true)"

export PATH="$business_repo"/cli/bin/:"$PATH"

# Create an admin.
BIM_BUSINESS_LEAD_TOKEN="$(bim-lead-create.sh \
                                | tee "$tmp_dir"/lead-create.json \
                                | jq -r .)"
export BIM_BUSINESS_LEAD_TOKEN

# Register the game server.
game_server_token="$(bim-game-server-create.sh \
                          --name ui-tests \
                          --description "UI tests" \
                          | tee "$tmp_dir"/game-server-create.json \
                          | jq -r .token)"

"$game_server" --console-log \
               --name "Bim! World's Server" \
               --enable-business \
               --business-token "$game_server_token" \
               --port "$game_server_port" \
               > "$tmp_dir"/game-server.out \
               2> "$tmp_dir"/game-server.err &

game_server_pid="$!"
echo "Server PID is $game_server_pid."

# Populate the business server to get something to show on the clients.
bim-game-feature-update.sh falling_blocks 50 \
                invisibility 250 \
                shield 250 \
                fences 500 \
                fog_of_war 1000

bim-game-feature-slots-update.sh 1 250

# Populate the shop.
bim-shop-update.sh coins_small 50 \
                coins_medium 250 \
                coins_large 500 \
                coins_huge 1000


initial_directory="$(pwd)"
working_directory=()

for script in "${scripts[@]}"
do
    if [[ ! -r "$script" ]]
    then
        echo "File does not exist or is not readable: '$script'." >&2
        exit 1
    fi

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

if [[ -n "$business_setup" ]]
then
    "$business_setup"
fi

i=0
for script in "${scripts[@]}"
do
    [[ -z "${working_directory[$i]}" ]] \
        || cd "${working_directory[$i]}"

    app_dir="$tmp_dir"/"$i"
    mkdir --parents "$app_dir"

    client_command=("${command[@]}"
                    --script "$script"
                    --app-dir "$app_dir")

    if [[ -n "$business_setup" ]]
    then
        "$business_setup" "$i" "$tmp_dir"/"$i"
    fi

    echo "Running" "${client_command[@]}"

    if (( use_gdb == 0 ))
    then
        "${client_command[@]}" > "$tmp_dir"/client.$i.out \
                               2> "$tmp_dir"/client.$i.err &

        p="$!"
        echo "Client #$i PID is $p."
        pids+=("$p")
        process_name["$p"]="Client #$i"
    else
        "${client_command[@]}"
    fi


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
