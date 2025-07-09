FROM ubuntu:24.04

RUN DEBIAN_FRONTEND=noninteractive \
    apt-get update \
    && apt-get install --assume-yes --no-install-recommends \
    ca-certificates \
    cron \
    curl \
    logrotate \
    && apt-get clean \
    && rm --recursive --force /var/lib/apt/lists/*
