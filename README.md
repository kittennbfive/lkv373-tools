# lkv373-tools
some random tools made while trying to reverse the lkv373-firmware

binfind is usefull for searching a binary file inside another one. It supports partial matches (with a configurable amount of different bytes).

bindiff is a really simple diff tool that outputs hexdumps like xxd and makes the differences visible in red. It does not have any fancy algorithms, just byte-to-byte comparison.

For more info just look at the code or execute without arguments.

compile with `gcc -std=c99 file.c`. tested on Linux only. no warranty.
