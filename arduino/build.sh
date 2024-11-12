#!/usr/bin/env bash
set -e
set -x


# Retrieve the directory where this script is located
scriptdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# Generate protobuf definitions

(cd "$scriptdir" && python3 "$scriptdir/nanopb/generator/nanopb_generator.py" "protocol.proto")

# Get the fqbn by retrieving the second last column of the board list using awk
fqbn=$(arduino-cli board list | tail -n 2 | head -n 1 | awk '{print $(NF-1)}')

# Compile the sketch
arduino-cli compile --libraries "$scriptdir" --fqbn $fqbn $scriptdir
