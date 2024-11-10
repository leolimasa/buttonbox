use std::time::Duration;

use serialport;

const STX: u8 = 0x02; // Start of text
                    
pub fn detect() -> Result<Vec<String>, Box<dyn std::error::Error>> {
    // TODO send a message to each device and try a handshake
    let ports = serialport::available_ports()?;
    Ok(ports.into_iter().map(|p| { p.port_name }).collect())
}

#[derive(Debug)]
pub enum SerialError {
    Connect(serialport::Error),
    MessageStart(std::io::Error),
    MessageRead(std::io::Error),
    StringParsing(std::string::FromUtf8Error),
    InvalidMessageType(u8),
}

pub fn connect(port_name: &str) -> Result<Box<dyn serialport::SerialPort>, SerialError> {
    serialport::new(port_name, 115200)
        .timeout(Duration::from_millis(100))
        .open()
        .or_else(|e| { Err(SerialError::Connect(e)) })
}

// Each message starts with STX. 
// Then the number of parameters as u8.
// Then, every parameter starts with 4 bytes (u32) that specifies the parameter length, 
// and the rest of the bytes are the parameter's contents.
//
// Message ends after all parameters are read.
pub fn read(port: &mut Box<dyn serialport::SerialPort>) -> Result<Vec<Vec<u8>>, SerialError> {

    // Read the first byte and checks if it is the start of a message
    let mut buf: Vec<u8> = vec![0; 1];
    port.read_exact(&mut buf).or_else(|e| { Err(SerialError::MessageStart(e)) })?;
    if buf[0] != STX {
        return Err(SerialError::MessageStart(std::io::Error::new(std::io::ErrorKind::InvalidData, "Invalid start of message")));
    }

    // Get number of parameters
    port.read_exact(&mut buf).or_else(|e| { Err(SerialError::MessageStart(e)) })?;
    let num_params = buf[0];

    // Read all parameters into a vector of parameters
    let mut params: Vec<Vec<u8>> = Vec::new();
    for _ in 0..num_params {
        // Read the first 4 bytes to get the parameter length
        port.read_exact(&mut buf).or_else(|e| { Err(SerialError::MessageRead(e)) })?;
        let param_len = u32::from_le_bytes([buf[0], buf[1], buf[2], buf[3]]) as usize;

        // Read the rest of the parameter
        let mut param: Vec<u8> = vec![0; param_len];
        port.read_exact(&mut param).or_else(|e| { Err(SerialError::MessageRead(e)) })?;
        params.push(param);
    }
    Ok(params)
}

pub enum Message {
    RotaryChange {
        index: i8,
        value: i8
    },
    ButtonPress {
        index: i8,
    },
    ButtonRelease {
        index: i8,
    },
    Debug {
        message: String,
    }
}

// Parse a message from a vector. Each item in the vector is a parameter. Each parameter
// is a byte array. The first byte is the message type. The rest of the bytes are 
// parsed into the corresponding parameter types.
pub fn parse(msg: Vec<Vec<u8>>) -> Result<Message, SerialError> {
    let msg_type = msg[0][0];
    match msg_type {
        b'R' => {
            let index = msg[1][0] as i8;
            let value = msg[2][0] as i8;
            Ok(Message::RotaryChange { index, value })
        },
        b'B' => {
            let index = msg[1][0] as i8;
            Ok(Message::ButtonPress { index })
        },
        b'L' => {
            let index = msg[1][0] as i8;
            Ok(Message::ButtonRelease { index })
        },
        b'D' => {
            let message = String::from_utf8(msg[1].clone()).or_else(|e| { Err(SerialError::StringParsing(e)) })?;
            Ok(Message::Debug { message })
        },
        _ => Err(SerialError::InvalidMessageType(msg_type))
    }
}
