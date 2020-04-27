THIS IS EXPERIMENTAL AND PROVIDED WITHOUT ANY WARRANTY!

DO NOT MESS WITH THIS UNLESS YOU HAVE A WAY TO REFLASH THE FIRMWARE EXTERNALLY!

licence: AGPL v3 or later

(c) 2020 by kitten_nb_five

This is a small "toolchain" to insert arbitrary code at arbitrary locations into block2.bin (that can be updated via the web-interface).

It can be used to insert multiple code-pieces at different adresses in one step.

The code to be injected must be written in assembly, use ONLY 4 byte instructions(!!) and follow this pattern:
-----------------
!!addr=0x<addr where to inject the following code>
push(m) <register(s) we want to use/modify>
<some code with possible calls>
pop(m)
j eof

<further blocks for different addresses, each ending with j eof>

<some common code like UART output functions>

.align 4 <-- this is important!
eof: <--- this MUST be at the end of the file and be named "eof"
-----------------

See example.asm.

Lines starting with a '!' are comments and ignored by the assembler. The Perlscript parses "!!addr=..." and fixes all the jump-addresses to work around a bug in GNU "as".

DO NOT USE GNU "as" DIRECTLY, you would get an endless loop. IT IS BROKEN. The Perlscript works around this.

The address where to inject code MUST contain a 4 byte instruction, not a 2 byte instruction! This is checked by the C-code.

How to use:
0) compile inject.c to inject and make it executable
1) write your assembly and save it into $file.asm
2) run ./assemble.pl $block2.bin $file.asm $output.bin
3) check the output of objdump file.asm
4) run ./make_binary.sh
5) flash $output.bin using web interface

Limitations / Risks:
THIS CAN BRICK YOUR LKV373.
We cannot put our code at the end of the existing code because this space is used as RAM. As a workaround the code is put somewhere where some strings are. This means that some output may be broken and also that we cannot insert too much code without breaking something else maybe. As i said, only use this if you can reflash the memory chip externally (using flashrom and a Bus Pirate for example).
You can put strings inside your assembly-code but you have to follow a specific sequence to make the output work. See example.asm.
If you call any functions from your code you MUST save and restore $r30 (also known as $lp).
