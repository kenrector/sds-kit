/* sds-trmeof.c - remove blank cards from end of Symbol punch output */

/*
 The Symbol assembler links with the Card Punch Binary Output
 Subroutine (Cat No. 020025/850046). That program defines a WEOF subroutine
 to write an end of file (blank card) on the output. This program will remove
 blank cards from a file without really testing that they are at the end
 of the file.  You could probably use tail instead.
 
 My version of the punch output subroutine does not output blanks cards.
 */

#include <stdio.h>
#include <string.h>


int main(int argc, char *argv[])
{
    FILE *infile;
    int i;
    int card[160];
    
    if (argc != 2) {
        printf("sds_trmeof - Remove blank cards from a file\n");
        printf("Usage: sds_trmeof  binary\n");
        exit(1);
    }
    infile = fopen(argv[1], "rb");
    if (!infile)
    {

        printf("Cannot open binfile: %s\n", argv[1]);
        return 1;
    }
    
    while (1) {
        i = fread(card,1,160,infile);
        if (i == 0)
            return 1;
        if (card[0] == 0x404040c0)
            return 1;
        fprintf(stderr,"   %8x   %8x\n",card[0], (card[0] & 0x700));
        
        if ((card[0] & 0x700) != 0x500) {
            fprintf(stderr,"non binary type record  %x\n",card[0]);
            return 1;
        }
        fwrite(card,4,40,stdout);
    }
}
