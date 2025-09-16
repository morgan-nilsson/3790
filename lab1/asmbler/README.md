lang spec:
2 registers - acc and idx

read [lit] - read a value from terminal to mem
- READ 10
write [lit] - write a value from mem to terminal
- WRITE 11

//mov acc [idx]
//- LOADIDX 23
//mov acc [lit]
//- LOAD 20
//mov acc lit
//- LOADIM 21
//mov idx [lit]
//- LOADX 22
//mov [idx] acc
//- STOREIDX 26
//mov [lit] acc
//- STORE 25

//add acc [idx]
//- ADDX 31
//add acc [lit]
//- ADD 30

//sub acc [idx]
//- SUBX 33
//sub acc [lit]
//- SUB 32

//mul acc [idx]
//- MULX 37
//mul acc [lit]
//- MUL 36

//div acc [idx]
//- DIVX 35
//div acc [lit]
//- DIV 34


//inc
//- INC 38

//dec
//- DEC 39

uses a standard label system
ie loop: 
jmp loop - unconditional jump
- BRANCH 40
jlz loop - jump if acc is zero
- BRANCHNEG 41
jez loop - jump if acc is equal to zero
- BRANCHEQ 42

swap - swap acc and idx
- SWAP 43

hlt lit - halt with printout 0000 to print nothing
- HALT 45

dw [lit] lit - define word at mem location

; for all line comments
; must be at the start of the line

idx - stack address

push acc

--idx
[idx] = acc

swp - acc = sp, idx = val
430000
dec - acc = sp - 1, idx = val
390000
swp - acc = val, idx = sp - 1
430000
mov [idx] acc
260000


pop acc
acc = [idx]
++idx

mov acc [idx]
230000
swp
430000
inc
380000

macro functions - need to be defined at top

MACRO movaround
STARTMACRO movaround
    mov eax, 
ENDMACRO

solve the following two problems :
1. Given two positive integers, compute their greatest common divisor. Use the algorithm shown here - https://en.wikipedia.org/wiki/Euclidean_algorithm
2. Given an array as input , find the largest amd smallest elements in the array