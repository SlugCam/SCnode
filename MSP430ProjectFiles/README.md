MSP430
========
Rather than going through TI's simplified eclipse IDE, msp430 projects are compiled
and programed through 2 commandline tools MSPGCC4 and MSPDEBUG. In this README I discuss
how to setup these tools and how to compile and run a msp430 project.

* * *

Setup
--------
Install all of these Packages through apt-get
* git-core	
* gcc-4.4
* texinfo
* patch
* libncurses5-dev
* zlibc
* zlib1g-dev
* libx11-dev
* libusb-dev
* libreadline6-dev
* binutils-msp430
* gcc-msp430
* gdb-msp430
* msp430-libc
* msp430mcu
* mspdebug
* srecord
* libsrecord-dev
* libgmp-dev

* * *

Compilation
--------
basic Compilation:
`msp430-gcc -Os -mmcu=msp430g2553 -o main.elf main.c`

more details see:  [msp430-gcc Manual](http://mspgcc.sourceforge.net/manual/)
* * *

mspdebug
--------
You must use mspdebug to program and run your projects.
Typically you run these commands:
1. `sudo mspdebug rf2500`
2. `erase`
3. `prog main.elf`
4. `run`

For using interupts and other options see this: [mspdebug Manual](http://mspdebug.sourceforge.net/manual.html)
