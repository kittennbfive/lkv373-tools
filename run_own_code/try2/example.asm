!example for injected code, prints out $IPC in hex over serial
!written by kitten_nb_five
!freenode #lkv373a
!licence: AGPL v3 or later
!NO WARRANTY! DO NOT MESS WITH THIS UNLESS YOU HAVE A WAY TO REFLASH THE DEVICE EXTERNALLY!

!due to a stupid bug in the GNU as you can't assemble this directly, you would get endless loops due to labels not working properly
!use assemble.pl instead

pushm $r0, $r1
push $r30
sethi $r0, hi20(str)
ori $r0, $r0, lo12(str)
mfusr $r1, $PC
add $r0, $r0, $r1
jal putstring

mfsr $r0, $IPC
jal puthex

pop $r30
popm $r0, $r1
j eof !!!

!--------------------------

puthex:
push $r30
pushm $r1, $r4
addi $r1, $r0, 0
movi $r2, 32
puthex_loop:
addi $r2, $r2, -4
srl $r3, $r1, $r2
andi $r3, $r3, 0x0f
slti $r4, $r3, 10
beqz $r4, puthex_1
addi $r3, $r3, '0'
j puthex_2
puthex_1:
addi $r3, $r3, 'A'-10
puthex_2:
addi $r0, $r3, 0
jal putchar
bnez $r2, puthex_loop
popm $r1, $r4
pop $r30
ret


putstring:
push $r1
push $r30
addi $r1, $r0, 0
loop:
lbi $r0, [$r1]
addi $r1, $r1, 1
beqz $r0, end
jal putchar
j loop
end:
pop $r30
pop $r1
ret

putchar:
pushm $r1, $r2
sethi $r1, 0x99600
ori $r1, $r1, 0x14
wait:
lbi $r2, [$r1]
andi $r2, $r2, 0x20
beqz $r2, wait
sethi $r1, 0x99600
ori $r1, $r1, 0x0
sbi $r0, [$r1]
sethi $r1, 0x99600
ori $r1, $r1, 0x14
wait2:
lbi $r2, [$r1]
andi $r2, $r2, 0x20
beqz $r2, wait2
popm $r1, $r2
ret

!--------------------------

str:
.asciz "val: " !.asciZ puts a '\0' at the end of the string like in C

!--------------------------

.align 4 !this is important!
eof:

