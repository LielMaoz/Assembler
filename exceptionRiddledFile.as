; file ps.as 

.entry LOOP 
LABEL:.entry LENGTH 
.entry NEVERUSED
LABEL:.extern L3 
.extern W 
MAIN: mov S1.1 ,LENGTH
MAIN:
add r2,STR 
mov r1
prn #55a3
prn #553
try
LOOP: jmp #5
macro m1 
 inc K
 mov S1.2 ,r3
endmacro 
prn #-5 
sub r1, r4
mov s1.4, r7
m1 
bne NotExist 
END: hlt 
STR: .string "abcdef" 
LENGTH: .data 6,-9,15 
K: .data b 
S1: .struct 8, "ab" 