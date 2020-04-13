#! /bin/sh
#we need to start .text at 0x04 because we add an instruction (=4 bytes) before the code from GCC to initialize the stack pointer. change this if needed
nds32le-elf-gcc test.c -nostdlib -mno-ext-perf -mno-ext-string -mno-v3push -march=v2 -Wl,--section-start=.text=0x04 -Wl,--entry=0 && nds32le-elf-objcopy -d binary a.out _test.bin && cat set_SP.bin _test.bin >test.bin && rm _test.bin a.out && nds32le-elf-objdump -b binary -m nds32 -D test.bin
