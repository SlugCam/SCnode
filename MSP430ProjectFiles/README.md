MSP430
========
Rather than going through TI's simplified eclipse IDE, SlugCam's firmware was developed, compiled,
and programed through the [Energia GUI](http://energia.nu/download/). Using this environment we could
take advantage of the much more readible macros and the overall development process is similar to the much
more user friend arduino platform. We do take advantage of the third party RTC library and the original
msp430 syntax when needed. Therefor make sure to include and import the nessecary header files along with
the relayContro ino file.


* * * WARNING DEPRECATED INSTRUCTIONS
If you choose to use the command line compiler you will need to make changes to the firmware source files.
Currently they do not compile, because of the adoption of the Energia library. However if you do wish to make 
changes follow these instructions:

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
