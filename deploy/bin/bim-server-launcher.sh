#!/bin/bash

set -euo pipefail

cp --recursive \
   /opt/bim/etc/logrotate.d \
   /opt/bim/etc/cron.weekly \
   /etc/

find /opt/bim/etc/cron.weekly/ -type f \
    | while read -r f
do
    chmod a+x /etc/cron.weekly/"$(basename "$f")"
done

[[ -f /opt/bim/persistent/GeoLite2-Country.mmdb ]] \
    || run-parts /etc/cron.weekly/ 2>&1 \
    || true

/opt/bim/bin/bim-server \
    --config /opt/bim/etc/bim/server-config.json \
    --log-file /opt/bim/persistent/log/bim-server.txt \
    --port "$1" \
    >> /opt/bim/persistent/log/bim-server.stdout.txt \
    2>> /opt/bim/persistent/log/bim-server.stderr.txt
