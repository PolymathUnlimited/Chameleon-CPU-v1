This repository contains the Logisim files and c++ assembler files for my Chameleon v1 CPU.

In order to use the assembler, download the c++ file and compile it using the compiler of your choice.  Then, simply run the assembler and choose an assembly file to target as prompted.  Make sure that the .asm file you want to assemble is in the same file directory as the assembler so it can find it.  I have included a simple hello world assembly program to test the assembler with, as well as both the binary and hexadecimal output the assembler should produce.  The assembler will also output hex code into the console that you can copy and then paste into the ROM of the CPU in Logisim.  While the assembler does technically work, it is very basic, and therefore leaves much to be desired.  I will be improving it in the future.

To run programs, open the CPU in Logisim and then paste the hexadecimal machine code you want to run into the ROM.  Make sure that Logisim has ticks enabled, and set the tick frequency as high as it will go.  Then press the "HRD RST" button (located next to the text display).  This will cause the contents of ROM to get loaded into RAM, after which the program will start executing.  Often, you don't need to wait for the program counter to cycle through all 64k of address space when loading programs into RAM, so you can simply press "SFT RST" a short while after pressing "HRD RST" in order to begin program execution more quickly.

This is a prototype CPU, and as such there are a lot of improvements I am continually making to its design.  I am currently rebuilding it on my YouTube channel, stay tuned for future updates:

http://www.youtube.com/@PolymathUnlimited-du2hg
