use std::collections::VecDeque;

mod serial;

fn main() {
    println!("Connecting");
    let ports = serial::detect().unwrap();
    println!("Detected serial device {:?}", ports[0]);
    let mut port = serial::connect(&ports[0]).unwrap();
    println!("Connected to serial device {:?}", port);

    let mut last_4_bytes: VecDeque<u8> = VecDeque::with_capacity(4);

    loop {
        let read_result = serial::read_byte(&mut port);
        let Ok(byte) = read_result else {
            continue;
        };

        // Print byte in hex
        // println!("{:02X}", byte);

        // Add the byte to the last_4_bytes queue.
        last_4_bytes.push_back(byte);
        if last_4_bytes.len() > 4 {
            last_4_bytes.pop_front();
        }

        // If the last 4 bytes match the magic sequence, then we have a message.
        if last_4_bytes.len() == 4 
            && last_4_bytes[0] == 0x3d 
            && last_4_bytes[1] == 0x01 
            && last_4_bytes[2] == 0x6f 
            && last_4_bytes[3] == 0x31 {

            // Read the next byte to get the message length.
            let message_length = match serial::read_byte(&mut port) {
                Ok(byte) => byte,
                Err(e) => {
                    // clear last last_4_bytes and continue
                    println!("Error reading message length: {:?}", e);
                    last_4_bytes.clear();
                    continue;
                }
            };
           
            // Read the message
            let message = match serial::read_message(&mut port, message_length) {
                Ok(message) => message,
                Err(e) => {
                    // clear last last_4_bytes and continue
                    println!("Error reading message: {:?}", e);
                    last_4_bytes.clear();
                    continue;
                }
            };
            println!("{:?}", message);
        }



        // Prints params content in hex for each vec item

        // let msg = serial::parse(params).unwrap();
        //
        // match msg {
        //     serial::Message::RotaryChange { index, value } => {
        //         println!("RotaryChange: index: {}, value: {}", index, value);
        //     },
        //     serial::Message::ButtonPress { index } => {
        //         println!("ButtonPress: index: {}", index);
        //     },
        //     serial::Message::ButtonRelease { index } => {
        //         println!("ButtonRelease: index: {}", index);
        //     },
        //     serial::Message::Debug { message } => {
        //         println!("Debug: message: {}", message);
        //     }
        // }
    }
}
