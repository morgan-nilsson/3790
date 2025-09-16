mod simpletron;

use simpletron::Simpletron;
use std::io::Write;

fn main() {


    println!("*** Welcome to Simpletron V2! ***");
    println!("***");
    print!("Do you have a file that contains your SML program (Y/N) ?");
    std::io::stdout().flush().expect("Failed to flush stdout");

    let input = std::io::stdin();
    let mut buffer = String::new();
    input.read_line(&mut buffer).expect("Failed to read line");
    let from_file = buffer.trim().to_lowercase() == "y";

    let mut src_lines = Vec::new();

    if from_file {
        print!("Please enter the file name: ");
        std::io::stdout().flush().expect("Failed to flush stdout");
        let mut file_name = String::new();
        input.read_line(&mut file_name).expect("Failed to read line");
        let file_name= file_name.trim();

        match std::fs::read_to_string(file_name) {
            Ok(contents) => {
                for line in contents.lines() {
                    src_lines.push(line.to_string());
                }
            },
            Err(e) => {
                println!("Error reading file: {}", e);
                return;
            }
        }
    } else {
        // read from stdin ending when input == "go"
        let mut buffer = String::new();
        while buffer.trim().to_lowercase() != "go" {
            println!(":");
            buffer.clear();
            input.read_line(&mut buffer).expect("Failed to read line");
            let line = buffer.trim().to_string();
            if line.to_lowercase() != "go" {
                src_lines.push(line);
            }
            
        }

    }

    // if the line is empty fill zero
    let instrs: Vec<i32> = src_lines.iter().map(|line| {
        if line.trim().is_empty() {
            0
        } else {
            match line.trim().parse::<i32>() {
                Ok(num) => num,
                Err(_) => {
                    println!("Invalid instruction: {}", line);
                    0
                }
            }
        }
    }).collect();
        

    let mut simpletron = Simpletron::new(instrs);

    simpletron.add_extension_instruction(12, write_char_extention);
    simpletron.add_extension_instruction(0, assert_extention);

    simpletron.execute(vec![""]);

}

fn write_char_extention(simpletron: &mut Simpletron, _operand: i32) {
    let value = simpletron.get_memory(simpletron.get_acc());
    let c = (value as u8) as char;
    print!("{}", c);
    std::io::stdout().flush().expect("Failed to flush stdout");
}

fn assert_extention(simpletron: &mut Simpletron, operand: i32) {
    if operand != 0 {
        if simpletron.get_acc() != operand {
            simpletron.dump_regs();
            simpletron.dump_memory(0, 0);
            assert!(false, "Assert failed: ACC ({}) != operand ({})", simpletron.get_acc(), operand);
        }
    } else {
        if simpletron.get_acc() != simpletron.get_ix() {
            simpletron.dump_regs();
            simpletron.dump_memory(0, 0);
            assert!(false, "Assert failed: ACC ({}) != IX ({})", simpletron.get_acc(), simpletron.get_ix());
        }
    }
}