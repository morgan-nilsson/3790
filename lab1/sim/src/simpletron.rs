use std::io::Write;

const MEMORY_PAGES: usize = 100;
const MEMORY_WORDS_PER_PAGE: usize = 100;

const MAX_MEMORY: usize = MEMORY_PAGES * MEMORY_WORDS_PER_PAGE;

const MAX_WORD: i32 = 999999;
const MIN_WORD: i32 = -999999;

// i32 = operand
type InstHandler = fn(&mut Simpletron, i32);
const INSTR_HANDLER_COUNT: usize = 50;

pub struct Simpletron {
    memory: [[i32; MEMORY_WORDS_PER_PAGE]; MEMORY_PAGES],
    // accumulator
    acc: i32,

    // index register
    ix: i32,

    // instruction pointer
    ip: i32,

    // instruction register
    ir: i32,

    // instruction handlers
    handlers: [Option<InstHandler>; INSTR_HANDLER_COUNT],

    is_halted: bool,
}

impl Simpletron {
    pub fn new(instructions: Vec<i32>) -> Self {
        let mut simple = Simpletron {
            memory: [[0; MEMORY_WORDS_PER_PAGE]; MEMORY_PAGES],
            acc: 0,
            ix: 0,
            ip: 0,
            ir: 0,
            handlers: [
                None,           None,               None,               None,           None,       // 0
                None,           None,               None,               None,           None,       // 5
                Some(read),     Some(write),        None,               None,           None,       // 10
                None,           None,               None,               None,           None,       // 15
                Some(load),     Some(load_im),      Some(load_x),       Some(load_idx), None,       // 20
                Some(store),    Some(store_idx),    None,               None,           None,       // 25
                Some(add),      Some(add_x),        Some(sub),          Some(sub_x),    Some(div),  // 30
                Some(div_x),    Some(mul),          Some(mul_x),        Some(inc),      Some(dec),  // 35
                Some(branch),   Some(branch_neg),   Some(branch_zero),  Some(swap),     None,       // 40
                Some(halt),     None,               None,               None,           None        // 45
            ],
            is_halted: false,
        };

        // Copy instructions into memory
        for (i, &instr) in instructions.iter().enumerate() {
            if i < MEMORY_PAGES * MEMORY_WORDS_PER_PAGE {
                simple.set_memory(i as i32, instr);
            } else {
                eprintln!("Warning: Instruction at index {} exceeds memory bounds and will be ignored.", i);
            }
        }

        simple
    }

    pub fn execute_step(&mut self) {

        if self.is_halted {
            return;
        }

        let instr = self.get_memory(self.ip);

        self.set_ip(self.ip + 1);

        let opcode = read_opcode(instr);
        let operand = read_operand(instr);

        if opcode < 0 || opcode >= INSTR_HANDLER_COUNT as i32 {
            eprintln!("Error: Invalid opcode {}\nHalting", opcode);
            self.is_halted = true;
            return;
        }

        if let Some(handler) = self.handlers[opcode as usize] {
            handler(self, operand);
        } else {
            eprintln!("Error: Unimplemented opcode {}\nHalting", opcode);
            self.is_halted = true;
            return;
        }

    }

    pub fn get_acc(&self) -> i32 {
        self.acc
    }

    fn set_acc(&mut self, value: i32) {

        if value < MIN_WORD || value > MAX_WORD {
            eprintln!("Error: Accumulator overflow/underflow with value {}\nHalting", value);
            self.is_halted = true;
            return;
        }

        self.acc = value;
    }

    pub fn get_ix(&self) -> i32 {
        self.ix
    }

    fn set_idx(&mut self, value: i32) {

        if value < MIN_WORD || value > MAX_WORD {
            eprintln!("Error: Index register overflow/underflow with value {}\nHalting", value);
            self.is_halted = true;
            return;
        }

        self.ix = value;
    }

    fn set_ip(&mut self, value: i32) {

        if value < 0 || value >= MAX_MEMORY as i32 {
            eprintln!("Error: Instruction pointer out of bounds with value {}\nHalting", value);
            self.is_halted = true;
            return;
        }

        self.ip = value;
    }

    pub fn get_memory(&mut self, index: i32) -> i32 {

        if index < 0 || index >= MAX_MEMORY as i32 {
            eprintln!("Error: Memory access out of bounds at address {}\nHalting", index);
            self.is_halted = true;
            return 0;
        }

        let (page, offset) = calculate_page_address(index);

        self.memory[page as usize][offset as usize]
    }

    pub fn set_memory(&mut self, index: i32, value: i32) {

        if index < 0 || index >= MAX_MEMORY as i32 {
            eprintln!("Error: Memory access out of bounds at address {}\nHalting", index);
            self.is_halted = true;
            return;
        }

        if value < MIN_WORD || value > MAX_WORD {
            eprintln!("Error: Memory value overflow/underflow with value {}\nHalting", value);
            self.is_halted = true;
            return;
        }

        let (page, offset) = calculate_page_address(index);

        self.memory[page as usize][offset as usize] = value;
    }

    pub fn dump_regs(&self) {
        println!("REGISTERS:\n");
        println!("Accumulator:          {:+07}",        self.acc);
        println!("InstructionCounter:   {:+07}",        self.ip);
        println!("IndexRegister:        {:+07}",        self.ix);
        println!("operationCode:             {:02}",    read_opcode(self.ir));
        println!("operand:                {:+05}",      read_operand(self.ir));
    }

    pub fn dump_memory(&mut self, start_page: usize, end_page: usize) {
        if start_page >= MEMORY_PAGES || end_page >= MEMORY_PAGES || start_page > end_page {
            eprintln!("Error: Invalid memory range for dump: {} to {}", start_page, end_page);
            self.is_halted = true;
            return;
        }

        for page in start_page..=end_page {
            println!("Page {}:", page);
            println!("         0       1       2       3       4       5       6       7       8       9");
            for offset in 0..MEMORY_WORDS_PER_PAGE {
                if offset % 10 == 0 {
                    if offset != 0 {
                        println!();
                    }
                    print!("{:02} ", offset);
                }
                let address = page * MEMORY_WORDS_PER_PAGE + offset;
                let value = self.get_memory(address as i32);
                print!("{:+07} ", value);
            }
            println!();
        }
    }

    pub fn add_extension_instruction(&mut self, index: u32, handler: InstHandler) {
        assert!((index as usize) < INSTR_HANDLER_COUNT, "Instruction index out of bounds: {}", index);
        self.handlers[index as usize] = Some(handler);
    }
}


fn read_opcode(instr: i32) -> i32 {
    (instr / 10000).abs()
}

fn read_operand(instr: i32) -> i32 {
    let sign = if instr < 0 { -1 } else { 1 };
    let operand = (instr % 10000).abs();
    sign * operand
}

// page, offset
fn calculate_page_address(index: i32) -> (i32, i32) {
    return (index / MEMORY_WORDS_PER_PAGE as i32, index % MEMORY_WORDS_PER_PAGE as i32);
}


// READ=10 - Read a word from the terminal into a location whose address is the operand
fn read(simpletron: &mut Simpletron, operand: i32) {

    loop {

        print!("? ");
        std::io::stdout().flush().expect("Failed to flush stdout");
        let mut input = String::new();
        std::io::stdin().read_line(&mut input).expect("Failed to read line");
        let input = input.trim();
        print!("\n");

        match input.trim().parse::<i32>() {
            Ok(value) => {
                simpletron.set_memory(operand, value);
                break;
            },
            Err(_) => {
                eprintln!("Error: Invalid input. Please enter a valid integer.");
            }
        }
    }
    
}

// WRITE=11 - Write a word from the location whose address is the operand to the terminal
fn write(simpletron: &mut Simpletron, operand: i32) {

    let value = simpletron.get_memory(operand);
    print!("=> {}\n", value);

}

// LOAD=20 - Load a word from the memory location specified by the operand into the accumulator 
fn load(simpletron: &mut Simpletron, operand: i32) {

    let value = simpletron.get_memory(operand);
    simpletron.set_acc(value);
    
}

// LOADIM=21 - Load the operand into the accumulator 
fn load_im(simpletron: &mut Simpletron, operand: i32) {

    simpletron.set_acc(operand);

}

// LOADX=22 - Load word from the memory location specified by the operand into the index register 
fn load_x(simpletron: &mut Simpletron, operand: i32) {

    let value = simpletron.get_memory(operand);
    simpletron.set_idx(value);

}

// LOADIDX=23 - Load word from the memory location specified by index register into accumulator 
fn load_idx(simpletron: &mut Simpletron, _: i32) {

    let value = simpletron.get_memory(simpletron.ix);
    simpletron.set_acc(value);

}

// STORE=25 - Store a word from the accumulator into the memory location specified by the operand 
fn store(simpletron: &mut Simpletron, operand: i32) {

    simpletron.set_memory(operand, simpletron.acc);

}

// STOREIDX=26 - Store a word from the accumulator into a memory location specified by index register 
fn store_idx(simpletron: &mut Simpletron, _: i32) {

    simpletron.set_memory(simpletron.ix, simpletron.acc);

}

// ADD=30 - Add the word in memory whose address is the operand to the accumulator and leave result in accumulator ( ACC += MEM ) 
fn add(simpletron: &mut Simpletron, operand: i32) {

    let value = simpletron.get_memory(operand);
    simpletron.set_acc(simpletron.get_acc() + value);

}

// ADDX=31 - Add a word in memory whose address is stored in index register to the accumulator and leave result in accumulator 
fn add_x(simpletron: &mut Simpletron, _: i32) {

    let value = simpletron.get_memory(simpletron.ix);
    simpletron.set_acc(simpletron.get_acc() + value);

}

// SUBTRACT=32 - Subtract a word whose address stored in the operand from the accumulator and leave result in accumulator ( ACC -= MEM ) 
fn sub(simpletron: &mut Simpletron, operand: i32) {

    let value = simpletron.get_memory(operand);
    simpletron.set_acc(simpletron.get_acc() - value);

}

// SUBTRACTX=33 - Subtract a word whose address is stored in the index register from the accumulator and leave result in accumulator 
fn sub_x(simpletron: &mut Simpletron, _: i32) {

    let value = simpletron.get_memory(simpletron.ix);
    simpletron.set_acc(simpletron.get_acc() - value);

}

// DIVIDE=34 - Divide the accumulator by a word whose address stored in the operand and leave result in accumulator and lose the remainder.( ACC /= MEM ) 
fn div(simpletron: &mut Simpletron, operand: i32) {

    let value = simpletron.get_memory(operand);
    if value == 0 {
        eprintln!("Error: Division by zero at address {}\nHalting", operand);
        simpletron.is_halted = true;
        return;
    }
    simpletron.set_acc(simpletron.get_acc() / value);

}

// DIVIDEX=35 - Divide the accumulator by a word whose address is stored in the index register and leave result in accumulator and lose the remainder. 
fn div_x(simpletron: &mut Simpletron, _: i32) {

    let value = simpletron.get_memory(simpletron.ix);
    if value == 0 {
        eprintln!("Error: Division by zero at address in index register {}\nHalting", simpletron.ix);
        simpletron.is_halted = true;
        return;
    }

    simpletron.set_acc(simpletron.get_acc() / value);

}

// MULTIPLY=36 - Multiply the accumulator by a word from a specific location in memory and leave result in accumulator ( ACC *= MEM ) 
fn mul(simpletron: &mut Simpletron, operand: i32) {

    let value = simpletron.get_memory(operand);
    simpletron.set_acc(simpletron.get_acc() * value);

}

// MULTIPLYX=37 - Multiply the accumulator by a word whose address is stored in the index register and leave result in accumulator 
fn mul_x(simpletron: &mut Simpletron, _: i32) {

    let value = simpletron.get_memory(simpletron.ix);
    simpletron.set_acc(simpletron.get_acc() * value);

}

// INC=38 - Increase index register by 1 
fn inc(simpletron: &mut Simpletron, _: i32) {

    simpletron.set_idx(simpletron.get_ix() + 1);

}

// DEC=39 - Decrease index register by 1 
fn dec(simpletron: &mut Simpletron, _: i32) {

    simpletron.set_idx(simpletron.get_ix() - 1);

}

// BRANCH=40 - Branch to a specific location in memory, location address is in operand 
fn branch(simpletron: &mut Simpletron, operand: i32) {

    simpletron.set_ip(operand);

}

// BRANCHNEG=41 - Branch to a specific location in memory if accumulator is negative 
fn branch_neg(simpletron: &mut Simpletron, operand: i32) {

    if simpletron.acc < 0 {

        simpletron.set_ip(operand);

    }

}

// BRANCHZERO=42 - Branch to a specific location in memory if the accumulator is zero 
fn branch_zero(simpletron: &mut Simpletron, operand: i32) {

    if simpletron.acc == 0 {

        simpletron.set_ip(operand);

    }

}

// SWAP=43 - Swap contents of index register and accumulator 
fn swap(simpletron: &mut Simpletron, _: i32) {

    // xor swap 
    simpletron.acc ^= simpletron.ix;
    simpletron.ix ^= simpletron.acc;
    simpletron.acc ^= simpletron.ix;
    
}

// HALT=45 - Halt program dump register values and a range of pages. The starting page of the range is stored as the top 2 digits of the operand and the last page as the least significant 2 digits (core dump). 
fn halt(simpletron: &mut Simpletron, operand: i32) {
    simpletron.dump_regs();
    let start_page = (operand / 100).abs() as usize;
    let end_page = (operand % 100).abs() as usize;
    simpletron.dump_memory(start_page, end_page);
    println!("Program halted.");
    simpletron.is_halted = true;
}
