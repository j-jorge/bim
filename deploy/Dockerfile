FROM ubuntu:25.10

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive \
    apt-get install --assume-yes --no-install-recommends \
    ca-certificates \
    cron \
    curl \
    logrotate \
    && apt-get clean \
    && rm --recursive --force /var/lib/apt/lists/*

