use std::{collections::HashMap, string};

pub struct Asmbler;

#[derive(Debug, PartialEq)]
pub enum AsmblerError {
    InvalidInstruction,
    InvalidOperand,
    SyntaxError,
    LabelAlreadyDefined,
    MACROAlreadyDefined,
    MACRONotDefined,
    MemoryRequestOutOfBounds,
}

#[derive(PartialEq)]
pub struct AssembledContents {
    pub contents: String,
    pub results: String,
    pub flags: Vec<String>,
    pub errors: Vec<(AsmblerError, String, usize)>,
}

impl AssembledContents {
    pub fn has_errors(&self) -> bool {
        !self.errors.is_empty()
    }

    pub fn print_errors(&self) {
        for (error, line, line_num) in &self.errors {
            println!("Error on line {}: {:?} - {}", line_num + 1, error, line);
        }
    }
}

const MAX_MEM_ADDR: u32 = 9999;

impl Asmbler {
    pub fn assemble(contents: String, _flags: Vec<String>) -> AssembledContents {

        let mut result = AssembledContents {
            contents: contents.clone(),
            results: String::new(),
            flags: _flags,
            errors: Vec::new(),
        };

        let mut labels: HashMap<String, usize> = HashMap::new();
        // (label_name, line_index, opcode)
        let mut unresolved_labels: Vec<(String, usize, u32)> = Vec::new();

        let mut defined_words: HashMap<u32, i32> = HashMap::new();

        let mut result_lines: Vec<String> = Vec::new();

        let lines = contents.split('\n');

        // filter out empty lines and trim whitespace
        let mut lines: Vec<&str> = lines
            .filter(|line| !line.trim().is_empty())
            .map(|line| line.trim())
            .collect();

        let mut macros: HashMap<String, Vec<String>> = HashMap::new();

        // first pass: collect macros and delete macro definitions from lines
        let mut i = 0;
        loop {
            if i >= lines.len() {
                break;
            }

            let line = lines[i];
            let parts = line.split_whitespace().collect::<Vec<&str>>();
            if parts[0] == "STARTMACRO" {
                let mut macro_body: Vec<String> = Vec::new();
                loop {
                    if i >= lines.len() - 1 {
                        result.errors.push((AsmblerError::SyntaxError, line.to_string(), i));
                        break;
                    }
                    lines.remove(i);
                    if lines[i].starts_with("ENDMACRO") {
                        lines.remove(i);
                        break;
                    }
                    macro_body.push(lines[i].to_string());
                }
                macros.insert(parts[1].to_string(), macro_body);
            }
            i += 1;
        }

        // second pass: expand macros
        let mut i = 0;
        loop {
            if i >= lines.len() {
                break;
            }
            let line = lines[i];
            let parts = line.split_whitespace().collect::<Vec<&str>>();
            if parts[0] == "MACRO" {

                let macro_name = parts[1];
                let macro_body = macros.get(macro_name);
                if macro_body.is_none() {
                    result.errors.push((AsmblerError::MACRONotDefined, line.to_string(), i));
                    i += 1;
                    continue;
                }

                let macro_body = macro_body.unwrap();
                lines.remove(i);
                for (j, macro_line) in macro_body.iter().enumerate() {
                    lines.insert(i + j, macro_line);
                }
                continue;
            }
            i += 1;
        }

        for i in 0..lines.len() {
            let line = lines[i];
            // skip comments
            if line.starts_with(";") { continue; }

            // read the instruction
            let parts: Vec<&str> = line.split_whitespace().collect();

            match parts[0] {
                "read" => {

                    if parts.len() != 2 {
                        result.errors.push((AsmblerError::SyntaxError, line.to_string(), i));
                        continue;
                    }

                    let dest = parts[1];

                    if is_like_mem_addr(dest) {
                        let mem_addr = parse_mem_addr(dest);
                        if mem_addr.is_err() {
                            result.errors.push((AsmblerError::InvalidOperand, line.to_string(), i));
                            continue;
                        }
                        let mem_addr = mem_addr.unwrap();
                        if mem_addr > MAX_MEM_ADDR {
                            result.errors.push((AsmblerError::MemoryRequestOutOfBounds, line.to_string(), i));
                            continue;
                        }
                        result_lines.push(format_instruction(10, mem_addr as i32));
                    } else {
                        result.errors.push((AsmblerError::InvalidOperand, line.to_string(), i));
                    }

                }
                "write" => {
                    if parts.len() != 2 {
                        result.errors.push((AsmblerError::SyntaxError, line.to_string(), i));
                        continue;
                    }

                    let src = parts[1];

                    if is_like_mem_addr(src) {
                        let mem_addr = parse_mem_addr(src);
                        if mem_addr.is_err() {
                            result.errors.push((AsmblerError::InvalidOperand, line.to_string(), i));
                            continue;
                        }
                        let mem_addr = mem_addr.unwrap();
                        if mem_addr > MAX_MEM_ADDR {
                            result.errors.push((AsmblerError::MemoryRequestOutOfBounds, line.to_string(), i));
                            continue;
                        }
                        result_lines.push(format_instruction(11, mem_addr as i32));
                    } else {
                        result.errors.push((AsmblerError::InvalidOperand, line.to_string(), i));
                    }

                }
                // writec acc
                "writec" => {
                    if parts.len() != 1 {
                        result.errors.push((AsmblerError::SyntaxError, line.to_string(), i));
                        continue;
                    }

                    result_lines.push(format_instruction(12, 0));
                }
                "mov" => {
                    if parts.len() != 3 {
                        result.errors.push((AsmblerError::SyntaxError, line.to_string(), i));
                        continue;
                    }
                    let dest = parts[1];
                    let src = parts[2];

                    match (dest, src) {

                        //mov acc [idx]
                        //- LOADIDX 23
                        ("acc", "[idx]") => {
                            result_lines.push(format_instruction(23, 0));
                        }
                        //mov acc [lit]
                        //- LOAD 20
                        ("acc", src) if is_like_mem_addr(src) => {
                            match parse_mem_addr(src) {
                                Ok(addr) if addr <= MAX_MEM_ADDR => {
                                    result_lines.push(format_instruction(20, addr as i32));
                                }
                                _ => {
                                    result.errors.push((AsmblerError::InvalidOperand, line.to_string(), i));
                                    continue;
                                }
                            }
                        }
                        //mov acc lit
                        //- LOADIM 21
                        ("acc", src) if is_like_lit(src) => {
                            match parse_lit(src) {
                                Ok(addr) => {
                                    result_lines.push(format_instruction(21, addr));
                                }
                                Err(e) => {
                                    result.errors.push((e, line.to_string(), i));
                                    continue;
                                }
                            }
                        }
                        //mov idx [lit]
                        //- LOADX 22
                        ("idx", src) if is_like_mem_addr(src) => {
                            match parse_mem_addr(src) {
                                Ok(addr) if addr <= MAX_MEM_ADDR => {
                                    result_lines.push(format_instruction(22, addr as i32));
                                }
                                _ => {
                                    result.errors.push((AsmblerError::InvalidOperand, line.to_string(), i));
                                    continue;
                                }
                            }
                        }
                        //mov [idx] acc
                        //- STOREIDX 26
                        ("[idx]", "acc") => {
                            result_lines.push(format_instruction(26, 0));
                        }
                        //mov [lit] acc
                        //- STORE 25
                        (dest, "acc") if is_like_mem_addr(dest) => {
                            match parse_mem_addr(dest) {
                                Ok(addr) if addr <= MAX_MEM_ADDR => {
                                    result_lines.push(format_instruction(25, addr as i32));
                                }
                                _ => {
                                    result.errors.push((AsmblerError::InvalidOperand, line.to_string(), i));
                                    continue;
                                }
                            }
                        }
                        
                        _ => {
                            result.errors.push((AsmblerError::InvalidOperand, line.to_string(), i));
                            continue;
                        }
                    }
                }
                "add" => {

                    if parts.len() != 3 {
                        result.errors.push((AsmblerError::SyntaxError, line.to_string(), i));
                        continue;
                    }

                    let dest = parts[1];
                    let src = parts[2];

                    match (dest, src) {

                        // add acc [idx]
                        // - ADDX 31
                        ("acc", "[idx]") => {
                            result_lines.push(format_instruction(31, 0));
                        }

                        // add acc [lit]
                        // - ADD 30
                        ("acc", src) if is_like_mem_addr(src) => {
                            match parse_mem_addr(src) {
                                Ok(addr) if addr <= MAX_MEM_ADDR => {
                                    result_lines.push(format_instruction(30, addr as i32));
                                }
                                _ => {
                                    result.errors.push((AsmblerError::InvalidOperand, line.to_string(), i));
                                    continue;
                                }
                            }
                        }

                        _ => {
                            result.errors.push((AsmblerError::InvalidOperand, line.to_string(), i));
                            continue;
                        }
                        
                    }
                }
                "sub" => {

                    if parts.len() != 3 {
                        result.errors.push((AsmblerError::SyntaxError, line.to_string(), i));
                        continue;
                    }

                    let dest = parts[1];
                    let src = parts[2];

                    match (dest, src) {

                        // sub acc [idx]
                        // - SUBX 33
                        ("acc", "[idx]") => {
                            result_lines.push(format_instruction(33, 0));
                        }

                        // sub acc [lit]
                        // - SUB 32
                        ("acc", src) if is_like_mem_addr(src) => {
                            match parse_mem_addr(src) {
                                Ok(addr) if addr <= MAX_MEM_ADDR => {
                                    result_lines.push(format_instruction(32, addr as i32));
                                }
                                _ => {
                                    result.errors.push((AsmblerError::InvalidOperand, line.to_string(), i));
                                    continue;
                                }
                            }
                        }

                        _ => {
                            result.errors.push((AsmblerError::InvalidOperand, line.to_string(), i));
                            continue;
                        }
                        
                    }

                }
                "mul" => {

                    if parts.len() != 3 {
                        result.errors.push((AsmblerError::SyntaxError, line.to_string(), i));
                        continue;
                    }

                    let dest = parts[1];
                    let src = parts[2];

                    match (dest, src) {

                        // mul acc [idx]
                        // - MULX 37
                        ("acc", "[idx]") => {
                            result_lines.push(format_instruction(37, 0));
                        }

                        // mul acc [lit]
                        // - MUL 36
                        ("acc", src) if is_like_mem_addr(src) => {
                            match parse_mem_addr(src) {
                                Ok(addr) if addr <= MAX_MEM_ADDR => {
                                    result_lines.push(format_instruction(36, addr as i32));
                                }
                                _ => {
                                    result.errors.push((AsmblerError::InvalidOperand, line.to_string(), i));
                                    continue;
                                }
                            }
                        }

                        _ => {
                            result.errors.push((AsmblerError::InvalidOperand, line.to_string(), i));
                            continue;
                        }
                    }
                }
                "div" => {

                    if parts.len() != 3 {
                        result.errors.push((AsmblerError::SyntaxError, line.to_string(), i));
                        continue;
                    }

                    let dest = parts[1];
                    let src = parts[2];

                    match (dest, src) {

                        // div acc [idx]
                        // - DIVX 35
                        ("acc", "[idx]") => {
                            result_lines.push(format_instruction(35, 0));
                        }

                        // div acc [lit]
                        // - DIV 34
                        ("acc", src) if is_like_mem_addr(src) => {
                            match parse_mem_addr(src) {
                                Ok(addr) if addr <= MAX_MEM_ADDR => {
                                    result_lines.push(format_instruction(34, addr as i32));
                                }
                                _ => {
                                    result.errors.push((AsmblerError::InvalidOperand, line.to_string(), i));
                                    continue;
                                }
                            }
                        }

                        _ => {
                            result.errors.push((AsmblerError::InvalidOperand, line.to_string(), i));
                            continue;
                        }
                    }
                }
                "inc" => {

                    if parts.len() != 1 {
                        result.errors.push((AsmblerError::SyntaxError, line.to_string(), i));
                        continue;
                    }
                    result_lines.push(format_instruction(38, 0));

                }
                "dec" => {

                    if parts.len() != 1 {
                        result.errors.push((AsmblerError::SyntaxError, line.to_string(), i));
                        continue;
                    }
                    result_lines.push(format_instruction(39, 0));

                }
                // labels
                s if s.ends_with(':') => {

                    let label_name = &s[..s.len()-1];
                    if labels.contains_key(label_name) {
                        result.errors.push((AsmblerError::LabelAlreadyDefined, line.to_string(), i));
                        continue;
                    }
                    labels.insert(label_name.to_string(), result_lines.len());

                }
                "jmp" => {
                    if parts.len() != 2 {
                        result.errors.push((AsmblerError::SyntaxError, line.to_string(), i));
                        continue;
                    }

                    let label_name= parts[1];

                    let label_addr = match labels.get(label_name) {
                        Some(&addr) => addr,
                        None => {
                            result_lines.push(String::new());
                            unresolved_labels.push((label_name.to_string(), result_lines.len() - 1, 40));
                            continue;
                        }
                    };

                    if label_addr > MAX_MEM_ADDR as usize {
                        result.errors.push((AsmblerError::MemoryRequestOutOfBounds, line.to_string(), i));
                        continue;
                    }

                    result_lines.push(format_instruction(40, label_addr as i32));
                }
                "jlz" => {

                    if parts.len() != 2 {
                        result.errors.push((AsmblerError::SyntaxError, line.to_string(), i));
                        continue;
                    }

                    let label_name= parts[1];

                    let label_addr = match labels.get(label_name) {
                        Some(&addr) => addr,
                        None => {
                            result_lines.push(String::new());
                            unresolved_labels.push((label_name.to_string(), result_lines.len() - 1, 41));
                            continue;
                        }
                    };

                    if label_addr > MAX_MEM_ADDR as usize {
                        result.errors.push((AsmblerError::MemoryRequestOutOfBounds, line.to_string(), i));
                        continue;
                    }

                    result_lines.push(format_instruction(41, label_addr as i32));
                }
                "jez" => {

                    if parts.len() != 2 {
                        result.errors.push((AsmblerError::SyntaxError, line.to_string(), i));
                        continue;
                    }

                    let label_name= parts[1];

                    let label_addr = match labels.get(label_name) {
                        Some(&addr) => addr,
                        None => {
                            result_lines.push(String::new());
                            unresolved_labels.push((label_name.to_string(), result_lines.len() - 1, 42));
                            continue;
                        }
                    };

                    if label_addr > MAX_MEM_ADDR as usize {
                        result.errors.push((AsmblerError::MemoryRequestOutOfBounds, line.to_string(), i));
                        continue;
                    }

                    result_lines.push(format_instruction(42, label_addr as i32));

                }
                "swap" => {
                    if parts.len() != 1 {
                        result.errors.push((AsmblerError::SyntaxError, line.to_string(), i));
                        continue;
                    }
                    result_lines.push(format_instruction(43, 0));
                }
                "hlt" => {
                    if parts.len() != 1 {
                        result.errors.push((AsmblerError::SyntaxError, line.to_string(), i));
                        continue;
                    }
                    result_lines.push(format_instruction(45, 0));
                }
                "dw" => {
                    if parts.len() != 3 {
                        result.errors.push((AsmblerError::SyntaxError, line.to_string(), i));
                        continue;
                    }

                    let dest = parts[1];
                    let src = parts[2];

                    if !is_like_mem_addr(dest) {
                        result.errors.push((AsmblerError::InvalidOperand, line.to_string(), i));
                        continue;
                    }

                    let mem_addr = parse_mem_addr(dest);
                    if mem_addr.is_err() {
                        result.errors.push((AsmblerError::InvalidOperand, line.to_string(), i));
                        continue;
                    }
                    let mem_addr = mem_addr.unwrap();
                    if mem_addr > MAX_MEM_ADDR {
                        result.errors.push((AsmblerError::MemoryRequestOutOfBounds, line.to_string(), i));
                        continue;
                    }

                    let lit = parse_lit(src);
                    if lit.is_err() {
                        result.errors.push((AsmblerError::InvalidOperand, line.to_string(), i));
                        continue;
                    }
                    let lit = lit.unwrap();

                    defined_words.insert(mem_addr, lit);
                }
                "assert" => {
                    if parts.len() == 1 {
                        result_lines.push(format_instruction(0, 0));
                    } else if parts.len() == 2 {
                        let lit = parse_lit(parts[1]);
                        if lit.is_err() {
                            result.errors.push((AsmblerError::InvalidOperand, line.to_string(), i));
                            continue;
                        }
                        let lit = lit.unwrap();
                        result_lines.push(format_instruction(0, lit));
                    } else {
                        result.errors.push((AsmblerError::SyntaxError, line.to_string(), i));
                        continue;
                    }

                }
                _ => {
                    // handle unknown instructions
                    result.errors.push((AsmblerError::InvalidInstruction, line.to_string(), i));
                }
            }

        }

        result_lines.push(format_instruction(45, 0)); // always end with hlt

        // resolve unresolved labels
        for (label_name, line_index, opcode) in unresolved_labels {
            match labels.get(&label_name) {
                Some(&addr) => {
                    if addr > MAX_MEM_ADDR as usize {
                        result.errors.push((AsmblerError::MemoryRequestOutOfBounds, format!("Unresolved label: {}", label_name), 0));
                        continue;
                    }
                    result_lines[line_index] = format_instruction(opcode, addr as i32);
                }
                None => {
                    result.errors.push((AsmblerError::InvalidOperand, format!("Unresolved label: {}", label_name), 0));
                }
            }
        }

        // insert defined words into result lines
        for (&addr, &value) in &defined_words {
            if addr as usize >= result_lines.len() {
                result_lines.resize(addr as usize + 1, "+000000".to_string());
            }
            result_lines[addr as usize] = format_instruction(0, value);
        }

        result.contents = result_lines.join("\n");
        return result;
    }
}

fn is_like_mem_addr(text: &str) -> bool {
    text.starts_with('[') && text.ends_with(']') && text[1..text.len()-1].parse::<u32>().is_ok()
}

fn is_like_lit(text: &str) -> bool {
    text.parse::<i32>().is_ok()
}

fn parse_lit(text: &str) -> Result<i32, AsmblerError> {
    match text.parse::<i32>() {
        Ok(lit) => Ok(lit),
        Err(_) => Err(AsmblerError::InvalidOperand),
    }
}

fn parse_mem_addr(text: &str) -> Result<u32, AsmblerError> {
    if !is_like_mem_addr(text) {
        return Err(AsmblerError::SyntaxError);
    }
    let inner = &text[1..text.len()-1];
    match inner.parse::<u32>() {
        Ok(addr) => Ok(addr),
        Err(_) => Err(AsmblerError::InvalidOperand),
    }
}

fn format_instruction(opcode: u32, operand: i32) -> String {
    let sign = if operand < 0 { "-" } else { "+" };
    format!("{}{:0>2}{:0>4}", sign, opcode, operand.abs())
}