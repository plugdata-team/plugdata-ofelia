#!/bin/bash

export DEBIAN_FRONTEND=noninteractive
export DEBIAN_PRIORITY=critical
export PATH='/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin'
command /usr/bin/apt --yes --quiet --option Dpkg::Options::=--force-confold --option Dpkg::Options::=--force-confdef "$@"

alias apt=$(realpath ./.github/scripts/silent-apt.sh)