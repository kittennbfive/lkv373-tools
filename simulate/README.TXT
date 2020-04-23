LKV373-simulator

(c)2019-2020 by kitten_nb_five

licence: AGPL v3 or later

THIS PROGRAM IS PROVIDED WITHOUT ANY WARRANTY!

This is an attempt to implement a simulator for the LKV373 to help with reverse engineering and eventually the development of a FOSS-firmware.

This simulator is currently really incomplete, a lot of the peripherals are not or only partially simulated (we don't have a lot of informations and reversing is hard and prone to errors) and only a fraction of the instruction set of the Andes N10 is implemented.

THIS SIMULATOR IS EXPERIMENTAL AND BUGGY! Feedback and help with debugging (see main.c) would be really appreciated!

THIS SIMULATOR WILL COMPILE/WORK ON LINUX ONLY! Tested on Debian 10.

Dependencies: ../disassm and some standard Linux-stuff, see window.c and layout.pl (or just try and see if there are errors...). And of course lkv373.bin in the same directory (a simlink is fine too)

How to compile: ./make (This is a hack, i don't know makefiles...)

How to use: ./sim

Please read the fine manual "MANUAL.TXT".

Quick start:
type "read script" and see the magic happen. You will need to adjust the windows positions and sizes to fit your screen, then you can type "layout save lay" to save everything (this is buggy, see layout.pl, you can edit the file lay.layout manually to fix the values)

Known limitations:
see main.c

If the simulator crashes you can do "rm fifo_sim" to close all remaining windows.

Current statistics:
github.com/AlDanial/cloc v 1.81  T=0.04 s (1378.0 files/s, 157471.1 lines/s)
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
C                               24            853            268           3820
C/C++ Header                    26            208            186            419
Perl                             1             14              2             58
-------------------------------------------------------------------------------
SUM:                            51           1075            456           4297
-------------------------------------------------------------------------------
+the code in ../disassm, total >6000 LOC.


Feedback welcome. Have fun and be careful.
