# buttonbox

A circuit board for Arduino nano with HID buttons for flight simulators.

2x3 rotary encoders in a circuit board.

## Features

* Heading (heading hold)
* Altitude (FLC)
* Vertical speed (VS)
* Speed (IAS hold)
* Altimeter
* COM radio

## Future

The goal is to have a 2x4 rotary encoders board with an extra 8 buttons. 

The amount of switches is limited by the number of pins on the Arduino nano. An alternative to increase that without using a multiplexer is to wire the switches in a matrix.

Each rotary takes 3 buttons. With 8 rotaries, that's 24 buttons. 
