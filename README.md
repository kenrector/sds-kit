# SDS Software Kit
This is a collection of software for use with the simh SDS simulator.  It is provided in hopes of
exciting the interested of those curious about computing during
the 1960-1976 period.

## Introduction
The SDS 900 Series computers supported paper tape and card unit record
systems and the software in this kit predominatly uses these devices.

Included are a cross assembler used on a Mac to produce the included
Symbol assembler and tools to help manage and
manipulate this stuff; things like a binary object dump, a namelist
program and a program to retieve files from a PAL library file.

The SDS programs in the kit are files copied from a
9-track tape that was created by the SDS PAL library back in 1982.
This file is from the bitsaver.org archives at http://www.bitsavers.org/bits/SDS/9xx/masterLibrary/M300_19820128.tap.gz

The SDS basic paper tape loader program, 850644, is included in the kit as 
an example to experiment with the SDS recon and Symbol assembler 
programs.  

The work to put this kit together was done on a Mac.  I don't know what 
will happen using this stuff on other platforms


## Files

File | Description
----|----
850640 | SEMI-AUTOMATIC TYPEWRITER TEST
850644 | BINARY INPUT-BASIC PAPER TAPE LOADER
850645 | UNIVERSAL LOADER
850647 | ENCODED TO SYMBOLIC RECONSTRUCTOR(RECON) 
850648 | BINARY INPUT ONE CARD LOADER
850649 | BINARY INPUT-TWO CARD LOADER
850657 | CARD PUNCH TEST PROGRAM PACKAGE
850816 | 910/925 ALGOL 60 BASIC 4K SYSTEM
890548 | BINARY TO SYMBOLIC TRANSLATOR
cross assembler | Assemble SYMBOL programs on a MAC
fortransa | 920/930 FORTRAN II SYSTEM (STAND ALONE) 
symbol | Symbol assembler and command files
tests  | A magtape test, so far
tools  | Possibly useful tools

## PAL and Program Identification

The SDS/Xerox Program Availability List (PAL) manual contains a list of all available Xerox software products, user programs and programming manuals. The systems that control the distribution and ensure the updating of the items listed are also described. 

A copy of a PAL manual may be found at http://www.bitsavers.org/pdf/sds/Xerox_Program_Availability_List_May75.pdf

All available Xerox (SDS) software is identified by a unique six-digit number which is referred to as a program catalog number. This catalog number may encompass a series of programs if these programs are ordinarily ordered as a group (e.g., Sigma 2/3 Numerical Subroutine Package, Catalog No. 705546). When this group is made available as a package, only a single "cover number" is used to identify it.
Every program catalog number has a two-digit number appended to it which is referred to as a program element. It is in the form "xy" where x usually refers to the form (source, binary, etc.) and y usually refers to the media (printed, magnetic tape, etc.) of the program. For example, a "-11" is a printed description of the program and a "-84" is absolute binary cards. The descriptions of these di:gits are summarized below:

First Digit Meaning | Second Digit Meaning
---|---
0 Miscellaneous | 0 Miscellaneous
1 Description | 1 Printed
2 Relocatable Binary | 2 7 level paper tape
3 Source | 3 8 level paper tape
4 Compressed | 4 Cards
5 Listing | 5 7 track magnetic tape
6 Update | 6 9 track magnetic tape
7 Miscellaneous | 7 Disk pack
8 Absolute Binary | 8 Miscellaneous
9 Miscellaneous | 9 Miscellaneous

NOTE:
There are two permanently assigned program elements with the first digit zero. The designation of -·01 is assigned to technical bulletins and a -02 is assigned to an unpublished technical document.
 Not all of the possible elements are available for each program. Programs are usually distributed on the type of media that would be most suitable for the potential users. Refer to the Status List section for the available elements for each program. If an element is not listed, it may be ordered only on a Field Request; see Special Requests below.







