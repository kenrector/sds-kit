# Uitlity Programs

##files
File | Description
--|--
sds_9pal | Extract file or list contents of an SDS Pal Library 9 track tape.
sds_nm | List external defs from SDS Standard Binary Object files.
sds_bindmp | Dump SDS Standard Binary Object file to stdout.
sds_listtape | List contents of simh mag tape.
sds_libsplt | Split a Fortran library file composed of binary object files
sds_libmk | Create a Fortran Library file from files in the current directory.
mkdeck |  Convert file from ascii to Hollerith format.

## Usage
sds_9pal - Extract file or list contents of an SDS Pal Library 9 track tape.

    Usage:  sds_pal  -l | -x nnnn-tt tapefile outfile [>stdout]
        -l :  List all catalog numbers
        -x nnnnn-tt : Extract catalog number nnnnn element type tt

    Extracted files will be unpacked and written to outfile.

    Type 84 files contain column binary images in standard binary format,
    sometimes prefaced by an absolute binary loader.

    Type 34 files contain Hollerith encoded data.  The unpacked
    Hollerith images will be written to outfile and also to stdout as
    ASCII characters.
 
    Type 44 files contain compressed program data that can be uncompressed
    by the 'recon' program (850647).  Sometimes the compressed data may be
    preceeded by control card which can cause an error during uncompression.


    The PAL library tape file is in simh Magnetic Tape format copied from
    a 9 track SDS tape.  Thanks to Al Kassow of the CHM and bitsavers.org
    for doing that.

    The tape contains label files composed of EBCDIC characters, (the
    tapes were made on a Sigma 7) and program data files containing packed
    6bit data, such that threee 8bit bytes contain four 6bit characters,
    or one 24bit 9 Series word.
 
    There is one :LBL file at the beginning of the tape with the tape's
    PAL identifier.  Following the :LBL are program files,
    preceeded by :BOF labels containing the PAL program catalog number
    to identify the data file.

    Program files consist of multiple blocks,
    usually of 2048 bytes. Program files are organized as follows:

         number of records
         record sequence number within file
         length of record data to follow, usually 160 bytes, but
         some times less if the record spans blocks
         packed data bytes

    Records may span across blocks. A partial packed set will be continued
    as a new record with the same sequence number in the next block.  A
    packed sequence of bytes will be continued in the next block and remaining
    packed data will be present to complete the record, to make 160 bytes.
    

    Catalog numbers have a two digits suffix to indicate the data type.
    The Pal manual defines the types as:
        2x  Relocatable Binary
        3x  Source cards (Hollerith)
        4x  Compressed Source (encoded)
        5x  Listing
        8x  Absolute Binary
            where x == 4 indicates card medium.
            
    The hexdump for an example Source file (890548-34) is:
    0000000 10 62 10 62 10 62 10 62 10 62 10 62 10 62 10 62
    *
    0000070 10 62 10 62 10 62 10 62 10 62 10 62 10 62 40 40
    0000080 40 40 40 40 40 40 40 40 40 40 40 40 40 40 40 40
                                     etc.
    which converts to ascii characters:
    **************************************** blank ...


sds_nm - List external defs from SDS Standard Binary Object files.

    Usage: sds_nm bias file...
        bias is an octal value defining the origin of the first file in file...
        file... is a list of SDS Standard Binary Object files.

    Sds_nm will ignore records not in standard binary format, ie, the 4th
    character is not 05.

  
sds_bindmp - Dump SDS Standard Binary Object file to stdout.

    Usage: sds_bindmp file
        file is a standard binary object file.

    Sds_bindmp will note and skip over records not in Standard Binary format, ie, the 4th
    character is not 05.
    Sds_bindmp will print Fortran Library catalog cards that start with '^2'.
    Sds_bindmp will print Control Word information from a Standard Binary format record.



sds_listtape - List contents of simh mag tape.

    Usage: sds_listtape file
        file is a simh tape format file.

    Sds_listtape is an adaptaion of Rich Cornwell's listape program that ignores
    gap markers produced by the SDS magtape simulator.

sds_libsplt - Split a Fortran library file composed of binary object files
   seperated by catalog cards into individual files named
   according to the catalog card names.

   Usage: sds_libsplt  file
        'file' is a Fortran Library file composed of binary object files
   	seperated by catalog cards.  Object files are copied to individual 
        files in the current dirctory named according to the catalog 
	card names.  Catalog cards are copied into file 'ctl' in the current
        directory in the order of their occurence in 'file'.

sds_libmk - Create a Fortran Library file from files in the current directory.

   Usage: sds_libmk file
        Copy binary object files named by records in the catalog file 'ctl' in the
        order of their appearance into 'file'.  Preceed each binary file by it's record 
        in the catalog file.

mkdeck -  Convert file from ascii to Hollerith format.
    Usage: mkdeck -a file
    
       Specify -a option to convert from ASCII to Hollerith format.  Other options are
       available
    Mkdeck is copied here from Rich Cornwell's I7000 kit
