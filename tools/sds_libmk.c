/* sds_libmk - assemble a library file  */

/*
 copy cards from binary files referenced by records in the catalog file
   preceed each binary file by it's record in the catalog file
 
*/

/* created by Ken Rector, Nov 21, 2020   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



FILE *infile, *outfile, *ctfile;
char crbuffer[160];
unsigned short image[80];

const unsigned short     ascii_to_hol[128] = {
   /* Control                              */
    0xf000,0xf000,0xf000,0xf000,0xf000,0xf000,0xf000,0xf000,    /*0-37*/
   /*Control*/
    0xf000,0xf000,0xf000,0xf000,0xf000,0xf000,0xf000,0xf000,
   /*Control*/
    0xf000,0xf000,0xf000,0xf000,0xf000,0xf000,0xf000,0xf000,
   /*Control*/
    0xf000,0xf000,0xf000,0xf000,0xf000,0xf000,0xf000,0xf000,
   /*  sp      !      "      #      $      %      &      ' */
    0x000, 0x482, 0x006, 0x282, 0x442, 0x222, 0x800, 0x022,     /* 40 - 77 */
   /*   (      )      *      +      ,      -      .      / */
    0x222, 0x822, 0x422, 0x800, 0x242, 0x400, 0x842, 0x300,
   /*   0      1      2      3      4      5      6      7 */
    0x200, 0x100, 0x080, 0x040, 0x020, 0x010, 0x008, 0x004,
   /*   8      9      :      ;      <      =      >      ? */
    0x002, 0x001, 0x012, 0x40A, 0x80A, 0x042, 0x00A, 0x882,
   /*   @      A      B      C      D      E      F      G */
    0x022, 0x900, 0x880, 0x840, 0x820, 0x810, 0x808, 0x804,     /* 100 - 137 */
   /*   H      I      J      K      L      M      N      O */
    0x802, 0x801, 0x500, 0x480, 0x440, 0x420, 0x410, 0x408,
   /*   P      Q      R      S      T      U      V      W */
    0x404, 0x402, 0x401, 0x280, 0x240, 0x220, 0x210, 0x208,
   /*   X      Y      Z      [      \      ]      ^      _ */
    0x204, 0x202, 0x201, 0x812, 0x20A, 0x412, 0x406, 0x082,
   /*   `      a      b      c      d      e      f      g */
    0x212, 0x900, 0x880, 0x840, 0x820, 0x810, 0x808, 0x804,     /* 140 - 177 */
   /*   h      i      j      k      l      m      n      o */
    0x802, 0x801, 0x500, 0x480, 0x440, 0x420, 0x410, 0x408,
   /*   p      q      r      s      t      u      v      w */
    0x404, 0x402, 0x401, 0x280, 0x240, 0x220, 0x210, 0x208,
   /*   x      y      z      {      |      }      ~    del*/
    0x204, 0x202, 0x201, 0x206, 0x806,0xf000,0xf000,0xf000
};

unsigned char        parity_table[64] = {
    /* 0    1    2    3    4    5    6    7 */
    0000, 0100, 0100, 0000, 0100, 0000, 0000, 0100,
    0100, 0000, 0000, 0100, 0000, 0100, 0100, 0000,
    0100, 0000, 0000, 0100, 0000, 0100, 0100, 0000,
    0000, 0100, 0100, 0000, 0100, 0000, 0000, 0100,
    0100, 0000, 0000, 0100, 0000, 0100, 0100, 0000,
    0000, 0100, 0100, 0000, 0100, 0000, 0000, 0100,
    0000, 0100, 0100, 0000, 0100, 0000, 0000, 0100,
    0100, 0000, 0000, 0100, 0000, 0100, 0100, 0000
};



int main (int argc, char *argv[]) {
    int i;
    char ifname[8];
    char out[160];
    int j;
    
    outfile = fopen(argv[1], "w");
    if (!outfile) {
        printf("Cannot open library output file: %s\n", argv[1]);
        exit (1);
    }
    
    ctfile = fopen("ctl","r");
    if (!ctfile) {
        printf("Cannot open catalog file: ctl \n");
        exit (1);
    }
    do {
        for (i = 0; i < 80; i++)
            crbuffer[i] = fgetc(ctfile);
        for (i = 0;i < 6;i++) {
            ifname[i] = crbuffer[8+i];
            if (ifname[i] == ' ')
                break;
        }
        ifname[i] = '\0';
        infile = fopen(ifname,"r");
        if (!infile) {
            printf("Cannot open input file %s\n",ifname);
            exit (1);
        }
        
        for (i = 0; i < 80; i++)
            image[i] = ascii_to_hol[crbuffer[i]];
        
        /* Fill buffer */
        for (i = 0; i < 80; i++) {
            unsigned short int  col = image[i];
            out[i*2] = (col >> 6) & 077;
            out[i*2+1] = col & 077;
        }
        
        /* Now set parity */
        for (i = 0; i < 160; i++)
            out[i] |= 0100 ^ parity_table[(int)out[i]];
        out[0] |= 0x80;     /* Set record mark */
        i = 160;
        fwrite(out, 1, i, outfile);
        
        /* copy the binary file */
        do {
            for (i = 0; i < 160; i++) {
                j = fgetc(infile);
                if (feof(infile))
                    break;
                fputc(j,outfile);
            }
        } while(!feof(infile));
        j = 0;
    } while(!feof(ctfile));
}
