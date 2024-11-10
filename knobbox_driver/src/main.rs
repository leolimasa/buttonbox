mod serial;

fn main() {
    println!("Connecting");
    let ports = serial::detect().unwrap();
    println!("Detected serial device {:?}", port);
    let port = serial::connect(&ports[0]).unwrap();
    println!("Connected to serial device {:?}", port);
    
    loop {
        let params = serial::read(&port).unwrap();

        // Prints params content in hex for each vec item
        for param in &params {
            print!("Param: ");
            for byte in param {
                print!("{:02X} ", byte);
            }
            println!();
        }

        let msg = serial::parse(params).unwrap();

        match msg {
            serial::Message::RotaryChange { index, value } => {
                println!("RotaryChange: index: {}, value: {}", index, value);
            },
            serial::Message::ButtonPress { index } => {
                println!("ButtonPress: index: {}", index);
            },
            serial::Message::ButtonRelease { index } => {
                println!("ButtonRelease: index: {}", index);
            },
            serial::Message::Debug { message } => {
                println!("Debug: message: {}", message);
            }
        }
    }
}
