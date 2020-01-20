#this is for "as", the GNU assembler contained in binutils
#NO WARRANTY! HIGHLY EXPERIMENTAL!
#AGPL v3 or later

.set printf, 0xcdc0

.set prog_org, 0x551a4
.set text_offset, 0x100
.set my_string, text_offset+prog_org

##prologue, save all registers
pushm $r0, $r31
movi $gp, 0 #this register is added to the call-register and make the code fail. just set it to zero, seems unused anyway


##our code
li $r1, my_string
movi $r2, 12345
call printf


##restore regs
popm $r0, $r31

##epilogue, adjust what we overwrote earlier
li $r15, printf

##go back to original code
ret $lp


.org text_offset
.ascii "owned! %d"
