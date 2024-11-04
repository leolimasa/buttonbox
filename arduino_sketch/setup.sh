#!/usr/bin/env bash
arduino-cli config init
arduino-cli core update-index

# Install the Arduino core for the identifier board from the Arduino CLI
platform=$(arduino-cli board list | tail -n 2 | head -n 1 | awk '{print $NF}')
arduino-cli core install $platform

# Install the libraries from the Arduino CLI

# https://github.com/PaulStoffregen/Encoder
arduino-cli lib install "Encoder"
curl -L -o /tmp/arduino_joystick.zip https://github.com/MHeironimus/ArduinoJoystickLibrary/archive/master.zip
export ARDUINO_LIBRARY_ENABLE_UNSAFE_INSTALL=true
arduino-cli lib install --zip-path /tmp/arduino_joystick.zip


