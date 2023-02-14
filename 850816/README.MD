# Algol 60 Compiler
## PAL Abstract
850816 9-SERIES  910/925 ALGOL 60 BASIC 4K SYSTEM
AUTHOR: XEROX
ABSTRACT:
TO COMPILE, LOAD AND EXECUTE ALGOL PROGRAMS FROM A FREE STANDING SYSTEM.
COMMENTS:
SOURCE LANGUAGE: META-SYMBOL. CONFIGURATION: 910/925 COMPUTER WITH AT LEAST 4K MEMORY,
TYPEWRITER AND PAPER TAPE I/0. SEE MANUAL NO. 900699.

## Introduction
This is a compressed source deck for the compiler.  It seems to be missing the 
interpreter and run time and the Algol Operations Manual.  The compiler will produce a 
program listing and a card punch file.  The PAL manual abstract for cover number includes
numbers for other parts but these are not found on the M300-1982128 tape

850816-44 is compressed source code and has 4 ^ type control cards preceeding the encoded deck.
The algol.si, .bo and .lo files were obtained by uncompressing and assembling this -44 file.

## Reference
http://www.bitsavers.org/pdf/sds/9xx/lang/900699C_Algol60_Ref_Nov66.pdf

## Problems
The original source code has TOPW 1 which caused an assmembly error. It was changed to TOPW.
There is no transfer address on the END card so the loader will halt after loading.

## Usage
bpt1 1	down, use card reader

bpt2 1	down, use card punch

bpt3 1  down, type source

bpt4 0  down, call objty before codout? Don't set this


The algol.ini configuration file allows one to do a compile and produce a listing on TTO and an
object deck on CP.

The prime.si file is copied from the example in the Algol Reference Manual.  The prime file is
a Hollerith card deck that can be attached to the CR device as compiler input.

The PAL manual describes 850690 as an ALGOL line printer library routine. Both the -24 (binary) 
and -34 (source) files are included here.  This seems to be a part of the Runtime Library and there
are no comments in the code mentioning Algol.

850690 900-SERIES ALGOL 60 EXT'D UNBUF LINE PRT. LIB ROUT.
AUTHOR: XEROX
ABSTRACT:
TO ALLOW USE OF UNBUFFERED LINE PRINTER IN ALGOL SYSTEM. THIIS LIBRARY PROGRAM IS LOADED WHEN OUTPUT TO THE LINE PRINTER IS CALLED FOR IN AN ALGOL PROGRAM. OUTPUT TO THE LINE PRINTER IS
COMMENTS:
SOURCE LANGUAGE:METASYMBOL COMPUTER CONFIGURATION: ANY XDS 900 SERIES COMPUTER WITH AN UNBUFFERED PRINTER (XDS MODEL NO. 9372).
850690-34  line printer library routine, source cards
850690-24  line printer library routine, binary
