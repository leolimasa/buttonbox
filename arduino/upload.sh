#!/usr/bin/env bash
set -e
set -x

# Retrieve the directory where this script is located
scriptdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# Get the fqbn by retrieving the second last column of the board list using awk
fqbn=$(arduino-cli board list | tail -n 2 | head -n 1 | awk '{print $(NF-1)}')

# Get the serial port of the board
port=$(arduino-cli board list | tail -n 2 | head -n 1 | awk '{print $1}')

# Upload the sketch
arduino-cli upload -p $port --fqbn $fqbn $scriptdir
