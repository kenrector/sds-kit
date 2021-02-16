/* sds_bindmp.c - Dump SDS Standard Binary Object file to stdout */

/* Created by Ken Rector, Nov. 7, 2020 */


#include <stdio.h>
#include <string.h>


FILE *infile, *outfile;
int btz;

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




unsigned int get24(){
    unsigned int i;
    
    i = fgetc(infile) & 077;
    i = i<<6 | (fgetc(infile) & 077);
    i = i<<6 | (fgetc(infile) & 077);
    i = i<<6 | (fgetc(infile) & 077);
    btz += 4;
    return(i);
}

int main(int argc, char *argv[])
{
    unsigned int i,j,n;
    unsigned int bin,type,cksm,cnt;
    unsigned int len;
    unsigned int cardno = 0;
    
    btz = 0;
    
    if (argc != 2)
    {
        printf("Usage: sds_bindmp binfile \n");
        printf("    Dump SDS Standard Binary Object file to stdout\n");
        printf("    Skip non-binary object file cards (col9-11 not 05)\n");
        printf("    Assumes each record is 80 columns\n");
        return 1;
    }
    
    infile = fopen(argv[1], "rb");
    if (!infile)
    {
        printf("Cannot open binfile: %s\n", argv[1]);
        return 1;
    }
    
    outfile = stdout;
    cardno = 0;
    
    do
    {
        cardno++;
        i = get24();
        if (feof(infile))
            break;
        if (i!=eof)
        {
            bin = (i >> 12) & 07;
            if (bin != 05) {
                if (i == 03200003) {
                    printf("Probable boostrap loader here\n");
                    for (j = 1; j< 40; j++)
                        i = get24();
                }
                else {
                    printf("\n%c",bcd_ascii[sim_hol_to_bcd(i>>12 &0xfff)]);
                    printf("%c",bcd_ascii[sim_hol_to_bcd(i &0xfff)]);
                    for (j = 1; j< 40; j++) {
                        i = get24();
                        printf("%c",bcd_ascii[sim_hol_to_bcd(i>>12 &0xfff)]);
                        printf("%c",bcd_ascii[sim_hol_to_bcd(i &0xfff)]);
                    }
                }
                printf("\n");
                continue;
            }
            type = i >> 21;
            cnt = (i >> 15) & 077;
            cksm = i & 07777;
            printf("%08o    ",i);
            len = 40;
            switch (type) {
                case 0:
                    printf("data ");
                    break;
                case 3:
                    printf("end ");
                    break;
                case 1:
                    printf("def/ref ");
                    break;
                default:
                    printf("unknown  %o\n",i);
            }
            printf("cnt = %o, chksm = %o %d\n",cnt,cksm, btz);
            
            if (type == 0 ) {
                i = get24();
                printf("%08o    load address = %08o\n",i,i);
                cnt--;
                len--;
            }
            for (j = 1; j<cnt;j++) {
                i = get24();
                printf("%08o \n",i);
                
            }
            for (;j < len;j++) {
                i = get24();
                printf("%08o \n",i);
            }
        }
        n++;
    } while(!feof(infile));
    
    fclose(infile);
    fclose(outfile);
}
