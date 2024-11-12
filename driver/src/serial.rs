use std::time::Duration;

use serialport;

pub fn detect() -> Result<Vec<String>, Box<dyn std::error::Error>> {
    // TODO send a message to each device and try a handshake
    let ports = serialport::available_ports()?;
    Ok(ports.into_iter().map(|p| { p.port_name }).collect())
}

#[derive(Debug)]
pub enum SerialError {
    Connect(serialport::Error),
    ByteRead(std::io::Error),
}

pub fn connect(port_name: &str) -> Result<Box<dyn serialport::SerialPort>, SerialError> {
    serialport::new(port_name, 115200)
        .timeout(Duration::from_millis(100))
        .open()
        .or_else(|e| { Err(SerialError::Connect(e)) })
}

pub fn read_byte(port: &mut Box<dyn serialport::SerialPort>) -> Result<u8, SerialError> {
    let mut buf: [u8; 1] = [0];
    port.read_exact(&mut buf).or_else(|e| { Err(SerialError::ByteRead(e)) })?;
    Ok(buf[0])
}
