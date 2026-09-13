# Latest LTS.
FROM ubuntu:latest

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive \
    apt-get install --assume-yes --no-install-recommends \
    ca-certificates \
    cron \
    curl \
    logrotate \
    && apt-get clean

