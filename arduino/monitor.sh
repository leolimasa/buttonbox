#!/usr/bin/env bash
set -e
set -x

# Get the fqbn by retrieving the second last column of the board list using awk
fqbn=$(arduino-cli board list | tail -n 2 | head -n 1 | awk '{print $(NF-1)}')

# Get the serial port of the board
port=$(arduino-cli board list | tail -n 2 | head -n 1 | awk '{print $1}')

# Monitor the serial port
arduino-cli monitor -p $port -b $fqbn
