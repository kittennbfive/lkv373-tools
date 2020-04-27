# lkv373-tools
some random tools made while trying to reverse the lkv373-firmware

NEW: simulate/ - My main work on this project. See README there.


binfind is usefull for searching a binary file inside another one. It supports partial matches (with a configurable amount of different bytes).

bindiff is a really simple diff tool that outputs hexdumps like xxd and makes the differences visible in red. It does not have any fancy algorithms, just byte-to-byte comparison.

For more info just look at the code or execute without arguments.
compile with `gcc -std=c99 file.c`.

beautify.pl, beautify_2.pl and beautify_3.pl try to  "beautify" the output of objdump -D. Just try it but don't expect too much.

usage: `perl beautify.pl>disassm_beautifed.txt && perl beautify_2.pl && perl beautify_3.pl`. Adjust filenames if needed.

strings is a helper for beautify, see code. ANSI-C.

disassm/ is an (incomplete) disassembler for this NDS32-architecture. written from scratch. see README there.

callgraphs/ makes visual representations of function calls starting at the function you want. based on disassm/, written in C and Perl. see README there.

call_lists/ lists all calls in the right order starting at some address. REALLY hacked together, based on callgraphs. mostly untested, probably really bad.

everything is under AGPL v3 or later, WITHOUT ANY WARRANTY and tested on Linux only.


General informations about the device are here: https://github.com/v3l0c1r4pt0r/lkv-wiki/wiki/LKV373
