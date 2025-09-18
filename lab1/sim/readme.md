This is a virtual enviroment to impliment the given instruction set architecture.

To build
```
cargo build
```

To run
```
cargo run
```

Once ran you can follow the prompts to run instructions.

You can also add ISA extentions. In the main.rs file there is an example of how to add these.
In this there are currently only two added extentions, one to write an ascii character to the screen and the other being an assertion

writec(12) - opcode 12
write the ascii char in the operand to the screen

assert - opcode 0
if operand == 0 
    then assert ACC == IX
else if operand != 0
    then assert ACC == operand
