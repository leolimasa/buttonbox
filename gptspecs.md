Below are specifications for a program that reads from an arduino serial device and prints the messages to the console. Write the program in Rust, and ensure that each message type has its own separate struct with the appropriate fields, there is a function to parse the raw bytes into the appropriate struct, and a function to print the struct to the console.

Specification:

# Serial protocol

* Each message is a collection of parameters
* A message starts with the STX (start of text) byte
* The message ends with the ETX (end of text) byte
* Parameters are delimited by the US (unit separator) byte

## Message types

Messages are identified by the first parameter. The first parameter is always a single byte that identifies the message type. 

The following message types are supported:

### Rotary change

Message type: `R`

Parameters:

* index, 4 byte int: The index of the rotary encoder that changed
* direction, 1 byte int: The direction of the change. 1 for clockwise, 0 for counter-clockwise

### Rotary click

Message type: `C`

Parameters:

* index, 4 byte int: The index of the rotary encoder that was clicked

# Device detection

Detect any boards that are connected via USB and support the serial protocol and have arduino as their name.
This process should work both in linux, macos, and windows. 

# Main loop

* Identify the serial device to read from, based on the algorithm above
  * If there is more than one device, the user should be prompted to select one 
* Read messages from the serial device, based on the serial protocol and message specification above
* Print the messages to the console
