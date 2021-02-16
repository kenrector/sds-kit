/* sds_libsplt.c - Split SDS Fortran library into individual files  */

/* Split a Fortran library file composed of binary object files
 seperated by catalog cards into individual files named
 according to the catalog card names.
 
 Create a file containig the catalog cards
 */

 /* Created by Ken Rector, Nov 21, 2020  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


FILE *infile, *outfile, *ctfile;
int btz;
int crbuffer[160];

#define eof 027657537

char bcd_ascii[64] = {
    ' ',    /* 0           - space */
    '1',    /* 1        1  - 1 */
    '2',    /* 2       2   - 2 */
    '3',    /* 3       21  - 3 */
    '4',    /* 4      4    - 4 */
    '5',    /* 5      4 1  - 5 */
    '6',    /* 6      42   - 6 */
    '7',    /* 7      421  - 7 */
    '8',    /* 8     8     - 8 */
    '9',    /* 9     8  1  - 9 */
    '0',    /* 10    8 2   - 0 */
    '=',    /* 11    8 21  - equal */
    '\'',    /* 12    84    - apostrophe */
    ':',    /* 13    84 1  - colon */
    '>',    /* 14    842   - greater than */
    '"',    /* 15    8421  - radical 017 {? */
    ' ',    /* 16   A      - substitute blank */
    '/',    /* 17   A   1  - slash */
    'S',    /* 18   A  2   - S */
    'T',    /* 19   A  21  - T */
    'U',    /* 20   A 4    - U */
    'V',    /* 21   A 4 1  - V */
    'W',    /* 22   A 42   - W */
    'X',    /* 23   A 421  - X */
    'Y',    /* 24   A8     - Y */
    'Z',    /* 25   A8  1  - Z */
    '#',    /* 26   A8 2   - record mark */
    ',',    /* 27   A8 21  - comma */
    '(',    /* 28   A84    - paren */
    '`',    /* 29   A84 1  - word separator */
    '\\',    /* 30   A842   - left oblique */
    '{',    /* 31   A8421  - segment mark */
    '-',    /* 32  B       - hyphen */
    'J',    /* 33  B    1  - J */
    'K',    /* 34  B   2   - K */
    'L',    /* 35  B   21  - L */
    'M',    /* 36  B  4    - M */
    'N',    /* 37  B  4 1  - N */
    'O',    /* 38  B  42   - O */
    'P',    /* 39  B  421  - P */
    'Q',    /* 40  B 8     - Q */
    'R',    /* 41  B 8  1  - R */
    '!',    /* 42  B 8 2   - exclamation */
    '$',    /* 43  B 8 21  - dollar sign */
    '*',    /* 44  B 84    - asterisk */
    ']',    /* 45  B 84 1  - right bracket */
    ';',    /* 46  B 842   - semicolon */
    '^',    /* 47  B 8421  - delta */
    '+',    /* 48  BA      - ampersand or plus */
    'A',    /* 49  BA   1  - A */
    'B',    /* 50  BA  2   - B */
    'C',    /* 51  BA  21  - C */
    'D',    /* 52  BA 4    - D */
    'E',    /* 53  BA 4 1  - E */
    'F',    /* 54  BA 42   - F */
    'G',    /* 55  BA 421  - G */
    'H',    /* 56  BA8     - H */
    'I',    /* 57  BA8  1  - I */
    '?',    /* 58  BA8 2   - question mark 032 */
    '.',    /* 59  BA8 21  - period */
    ')',    /* 60  BA84    - paren */
    '[',    /* 61  BA84 1  - left bracket 035 */
    '<',    /* 62  BA842   - less than 036 */
    '}'    /* 63  BA8421  - group mark 037 */
};


/* Returns the BCD of the hollerith code or 0x7f if error */
int sim_hol_to_bcd(short hol) {
    int  bcd;
    
    /* Convert 10,11,12 rows */
    switch (hol & 0xe00) {
        case 0x000:
            bcd = 0;
            break;
        case 0x200:
            if ((hol & 0x1ff) == 0)
                return 10;
            bcd = 020;
            break;
        case 0x400:
            bcd = 040;
            break;
        case 0x600: /* 11-10 Punch */
            bcd = 052;
            break;
        case 0x800:
            bcd = 060;
            break;
        case 0xA00: /* 12-10 Punch */
            bcd = 072;
            break;
        default: /* Double punch in 10,11,12 rows */
            return 0x7f;
    }
    
    hol &= 0x1ff;       /* Mask rows 0-9 */
    /* Check row 8 punched */
    if (hol & 0x2) {
        bcd += 8;
        hol &= ~0x2;
    }
    
    /* Convert rows 0-9 */
    while (hol != 0 && (hol & 0x200) == 0) {
        bcd++;
        hol <<= 1;
    }
    
    /* Any more columns punched? */
    if ((hol & 0x1ff) != 0)
        return 0x7f;
    return bcd;
}

unsigned int getascii(int i){
    int a;
    
    a = ((crbuffer[i] & 077) << 6) + (crbuffer[i+1] & 077);
    return(bcd_ascii[sim_hol_to_bcd(a)]);
}

int main(int argc, char *argv[]) {
    unsigned int i,j,n;
    int k;
    unsigned int cardno = 0;
    char numc[12];
    char ofname[64];

    
    btz = 0;
    
    infile = fopen(argv[1], "rb");
    if (!infile)
    {
        printf("Cannot open binfile: %s\n", argv[1]);
        return 1;
    }
    ctfile = fopen("ctl","w");
    for (j = 0; j < 160; j++) {
        crbuffer[j] = fgetc(infile) & 0xff;
    }
    cardno++;
    do {
        i = getascii(0);
        if (i != '^') {
            printf("Missing control card:  %d\n",cardno);
            for (j = 0; j < 160; j++) {
                printf("%02x ",crbuffer[j]);
                if (((j+1) % 0x10) == 0)
                    printf("\n");
            }
            printf("\n");
        }
        else {
            for (j = 0; j < 160; j+=2)
                fprintf(ctfile,"%c",getascii(j));
            for (j = 0; j < 10;j++)
                numc[j] = getascii(68+(2*j));
            numc[j] = '\0';
            n = atoi(numc) -2;
            
            for (j = 0; j < 7; j++) {
                ofname[j] = getascii(16+(2*j));
                if (ofname[j] == ' ') {
                    ofname[j] = '\0';
                    outfile = fopen(ofname,"w");
                    break;
                }
            }
                    
        }
        do {
            for (j = 0; j < 160; j++) {
                crbuffer[j] = fgetc(infile) & 0xff;
            }
            i = getascii(0);
            if (i == '^') {
                //fclose (outfile);
                break;        // new deck
            }
            
            if ((crbuffer[1] & 07) != 0x5) {
                printf("Not std binary\n");
                exit(1);
            }
            for (j = 0; j < 160; j++)
                fputc(crbuffer[j],outfile);
        } while(!feof(infile));
    } while(!feof(infile));
    fclose (ctfile);
    fclose (outfile);
}
