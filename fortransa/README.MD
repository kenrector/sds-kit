# Stand Alone Fortran II

## PAL Abstract
850957 9-SERIES 920/930 FORTRAN II SYSTEM (STAND ALONE) 
AUTHOR: XEROX
ABSTRACT:
THIS IS THE STAND-ALONE 920/930 FORTRAN-II PACKAGE CONSISTING OF COMPILER, LOADER
AND RUN-TIME/LIBRARY
COMMENTS:
SOURCE LANGUAGE:SYMBOL. SIZE 4096 DECIMAL. THIS PROGRAM COVERS 850938,850959,850960.
SEE MANUALS 900587, 900 SERIES FORTRAN II OPERATIONS, 900003. 900 SERIES FORTRAN II REFERENCE MANUAL AND 901048, 900 SERIES FORTRAN II TECHNICAL MANUAL. 
CONFIGURATION: ANY 920/930 COMPUTER.

## Introduction
The SDS Fortran II compiler and runtime system was developed by Digitek and was their first product.  The compiler is implemented in a virtual language using POPs, (Programmed Operators) which is a feature of the SDS 9 Series computers.  The virtual language implements a stack machine with operators designed for compiler implementation.  Digitek Fortran II was a major contribution to the success of both SDS and Digitek.

The 850957-84 file is composed of four sections delimited by Standard Binary Object End records and library catalog cards.  
You may obtain the four parts, the Compiler (fort), Loader (fldr),  Library (flib) and Runtime (frun), by splitting the
850957-84 file using head and tail after identifying the length of each section using tools/sds_bindmp.  These files have been included here for your convience.

1. The Compiler, fort, preceeded by a two card loader and terminated by an end record.
2. The Loader,fldr, preceeded by a two card loader and terminated by an end record.
3. The Library, flib, with 55 object decks, each preceeded by a catalog record and terminated by an end record (exept the last).  The catalog record is a Hollerith BCD image beginning with the characters ^2 folowed by spaces and a file name.  The last object deck has a catalog record with the file name SYS777 and has no end card.
4. The Runtime program, frun, preceeded by a two card loader and (supposedly) terminated by an end record.  (See problem 2. below)



## Reference Material
http://www.bitsavers.org/pdf/sds/9xx/lang/900003D_900_FORTRAN_II_Ref_Oct70.pdf
http://www.bitsavers.org/pdf/sds/9xx/lang/900587B_FORTRAN_II_OperationsMan_Jul66.pdf
http://www.bitsavers.org/pdf/sds/9xx/lang/900065C_900_4K_FORTRAN_II_Tech_Jan65.pdf

## Problems
There are two problems with the 850957-84 file extracted from both the M300_19820128.tap and M400_19820305.tap files.   

1, Three of the library files have out of order records.  It appears that these problems occured when copying the original card decks to tape, perhaps cards were dropped and then put back into the deck incorrectly as some were in reverse order.  I used xxd and vi to edit the files to straighten things out.  A corrected version of the library was saved as flib_corrected and the original, uncorrected version of the library is found in flib_original.

2, The runtime program is missing the last several data records and the end record.  The missing end record was corrected but I could not find the missing data records.  This file was saved as runtime_ldr.  A version of a runtime program based on the source code in the Fortran II Technical Reference Manual, (900065) is included as frun.si, frun.bo and frun.lo for experimenting.  This is notably different than the corrupted runtime program on the PAL trap but it will load and begin execution of the simple.si example.

## Usage

Consult the Fortran II Operations Manual, 900587, for operating instructions for compiling and loading programs.

This directory includes three simh command files for use with the Fortran II SA System.  

Use the fortran or fort_ascii command file to run the compiler with either Hollerith or ASCII format input.  The object deck will be saved on the card-punch file attached to the cp device.

Use the fortran_run command file to load and run a binary object deck loaded with files from the flib and the frun.bo runtime.  Since frun.bo is not preceeded by a loader the command file includes ldr_2crd preceeding frun.bo in the input deck. 

A simple HELLO WORLD example is included as hello.f with output from its execution in hello_sample.   Note that there is a halt at location 5 after the load map is printed.  Clear this halt by branching to location 1 to continue, then type c to continue after loading is completed to execute the program.  The program currently doesn't print the HELLO WORLD message. Maybe you can figure out why?

The program fiiexample.f is also included.

The Fortran II system depended on the operator to manage cards in the reader and punch.  The simulator does not close and flush input and output on those devices just because the program has halted so its up to you to reset devices or quit the simulator as necessary.
