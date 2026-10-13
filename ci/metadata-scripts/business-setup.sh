#!/bin/bash

set -euo pipefail

if (( $# == 0 ))
then
    bim-app-config-update.sh \
        legacy.enable_transfer 'true' \
        games.max_duration_for_short_game.seconds 0 \
        games.coins_per_victory 50 \
        games.coins_per_defeat 5 \
        games.coins_per_draw 0

    exit 0
fi

user_index="$1"
user_config_dir="$2"

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

cp "$script_dir"/preferences.json "$user_config_dir"/

device_id=00000000-0000-0000-0000-00000000000"$user_index"
echo "$device_id" > "$user_config_dir"/device-id.txt

do_curl()
{
    local path="$1"
    shift

    curl --fail --silent --show-error \
         "$BIM_BUSINESS_SERVER_URL"/"$path" "$@"
}

session_token="$(do_curl client/authenticate \
                         --header "Content-Type: application/json" \
                         --data '{"device_id": "'"$device_id"'"}' \
                         | jq -r .session_token)"

nickname=("John 🫶​" "ジョン 👍" "João 🙂" "Γιάννης ✨​")
do_curl client/account/update-nickname \
        --header "Authorization: $session_token" \
        --header "Content-Type: application/json" \
        --data '{"nickname": "'"${nickname[$user_index]}"'"}'
