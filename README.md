# CMPUT 379: Operating Systems
## Assignment 1: Fall 2016
Jessica Prieto 1412737

Dannick Pomerleau 1388804

## Compiling
make all

## Running
Driver program 1: ./bin/mem_mod1

Driver program 2: ./bin/mem_mod2

Driver program 3: ./bin/mem_mod3

## Regions

Here is an example output of the get_mem_regions:

'''
There are 14 regions
0x00000000 - 0x08047fff NO
0x08048000 - 0x0804afff RO
0x0804b000 - 0x0804bfff RW
0x0804c000 - 0xb7519fff NO
0xb751a000 - 0xb751afff RW
0xb751b000 - 0xb76c4fff RO
0xb76c5000 - 0xb76c8fff RW
0xb76c9000 - 0xb76dcfff NO
0xb76dd000 - 0xb76dffff RW
0xb76e0000 - 0xb7701fff RO
0xb7702000 - 0xb7702fff RW
0xb7703000 - 0xbfd86fff NO
0xbfd87000 - 0xbfda8fff RW
0xbfda9000 - 0xffffffff NO
'''

The regions will be referenced by numbers where the region starting at 0x00000000 is 0.
0: Kernel Memory Region

Program
1: The folowing region is divided in the following sections, 
the number following the name is the starting address of the subregion
    .interp           08048154
    .note.ABI-tag     08048168
    .note.gnu.build-i 08048188
    .gnu.hash         080481ac
    .dynsym           080481cc
    .dynstr           080482bc
    .gnu.version      08048358
    .gnu.version_r    08048378
    .rel.dyn          08048398
    .rel.plt          080483a0
    .init             08048408
    .plt              08048430
    .text             08048510 // the program code
    .fini             080490a4
    .rodata           080490b8 // constants
    .eh_frame_hdr     0804910c
    .eh_frame         08049158
    .init_array       0804af08
    .fini_array       0804af0c
    .jcr              0804af10
    .dynamic          0804af14
    .got              0804affc

2:
    .got.plt          0804b000 // dynamic library inderiction pointers
    .data             0804b040 // dynamic global initialized data
    .bss              0804b060 // dynamic global uninitialized data
3: Guard Region

Dynamic Libraries
libc
4: .text
5: .rodata
6: .data, .bss
7: Guard Region

ld
8: .text
9: .rodata
10: .data, .bss
11: Guard Region

The purpose of 4 - 11 was found using pmap on the program after it was stoped.

Stack
12: Stack
13: Guard Region

## Operations

* mem_mod1: mmap PAGE_SIZE of zeros

* mem_mod2: mmap 3 * PAGE_SIZE of zeros

* mem_mod3: mmap PAGE_SIZE of zeros RW then mprotect it to RO

In our driver program we get the systems PAGE_SIZE using sysconf. Then using that we mmap one page
of data. In mem_mod2 we mmap 3 pages of data.

## Observations
The programs where tested on the VM.

Most page boundaries change each time the program is executed. The random boundaries are to help protect from unwanted code execution due to buffer overflows. The reason that this works is because there is not a fixed distance between the heap and the stack. This is security through obscurity and does not guarantee that the system is safe from buffer overflows.

If you look at the region right before the stack you can see that it is set to NO. 
I believe that if you try to read or write in the page right before the stack the 
linux kernel will capture the segfault trap and allocate you a new page instead of 
signalling a segfault. The reason for this behaviour is because every function you 
call pushes registers on the stack. If you end up pushing registers over page 
boundaries the OS automatically gives you a new page because you don't want your 
program to crash on an arbitrary function call. This is useful because you don't 
want to a segfault to stop your memsort. 

# Credits
[1] Caf, Accepted Answer: How can I check whether a memory address is writable or not at runtime? 
Stack Overflow, Published 2013-01-21, Accessed 2016-09-24, 
http://stackoverflow.com/questions/14433468/how-can-i-check-whether-a-memory-address-is-writable-or-not-at-runtime

[2] Florent Bruneau, Memory - Part 2: Understanding Process Memory,
Intesec TechTalk, Published 2013-07-23, Accessed 2016-10-01,
https://techtalk.intersec.com/2013/07/memory-part-2-understanding-process-memory/
