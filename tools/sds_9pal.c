/* sds_9pal.c - Extract file from 9-track PAL library tape */

/* Created by Ken Rector, Feb. 3 2020  */

//  Extract file or list contents of an SDS Pal Library Tape.
//
//  The PAL library tape file is in SIMH Magnetic Tape format copied from
//  a 9 track SDS tape.  Thanks to Al Kassow of the CHM and bitsavers.org
//  for doing that.
//
//  Each file from the Pal tape contains labels and data.  Labels contain
//  EBCDIC characters, (the tapes were made on a Sigma 7) and data following
//  these contain packed 6bit data such that threee 8bit bytes contain four
//  6bit characters or 1 24bit 9 Series word.
//
//  There is one :LBL file at the beginning of the tape with the PAL identifier.
//
//  The :LBL file is followed by the program files preceeded by :BOF labels,
//  containing the PAL program catalog number to identify data file.
//
//  The data file following the :BOF label consists of multiple blocks, usually
//  2048 bytes.  Data files are organized as follows:
//
//  int number of records
//          int record sequence number within file
//          int length of record data to follow, usually 160 bytes, but
//          some times less if record spans blocks
//          char[length]  packed data bytes
//
//  Records may span across blocks. A partial packed set will be continued
//  as a new record with the same sequence number in the next block.  A
//  packed sequence of bytes will be continued in the next block and remaining
//  packed data will be present to complete the record, to make 160 bytes.
//
//  Catalog numbers are suffixed by two digits to indicate the data type.
//  The Pal manual defines the types as:
//      2x  Relocatable Binary
//      3x  Source cards
//      4x  Compressed Source (encoded)
//      5x  Listing
//      8x  Absolute Binary
//          where x = 4 indicates card medium.
//
//  A type -34 file contains Hollerith coded source records.
//  The hexdump for an example (890548-34) is:
//     0000000 10 62 10 62 10 62 10 62 10 62 10 62 10 62 10 62
//     *
//     0000070 10 62 10 62 10 62 10 62 10 62 10 62 10 62 40 40
//     0000080 40 40 40 40 40 40 40 40 40 40 40 40 40 40 40 40
//                                      etc.
//  which converts to ascii characters:
//     **************************************** blank ...
//
//
//  A type -44 file contains encoded data that was called
//  'compressed' by the SDS Metasymbol assembler.  This can
//  be uncompressed by the 'encode to symbol' program (850647).
//
//  The 'encode to symbol' program expects a keyboard input command to
//  define it's input output configuration.  This command takes the form
//  of '^XX' where the first X indicates the input device and the
//  second, the output device.  Errors can be deleted by typing '/' and
//  retyping the command from the beginning '^'.
//
//  Devices can be any of CPM, card, papertape and magtape, respecively.
//  Specify card input and mag tape output by '^CM'.
//  The mag tape output can be read using the simh listtape utility.
//
//
//


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


char ebcdic_to_ascii[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x09, 0x06, 0x07,     /* 00 - 1F */
    0x08, 0x05, 0x15, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x0A, 0x16, 0x17,  //10
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 20 - 3F */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 30
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    ' ',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 40 - 5F */
    0x00, 0x00, '`',  '.',  '<',  '(',  '+',  '|',
    '&',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //50
    0x00, 0x00, '!',  '$',  '*',  ')',  ';',  '~',
    '-',  '/',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 60 - 7F */
    0x00, 0x00, '^',  ',',  '%',  '_',  '>',  '?',
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //70
    0x00, 0x00, ':',  '#',  '@',  '\'', '=',  '"',
    0x00, 'a',  'b',  'c',  'd',  'e',  'f',  'g',      /* 80 - 9F */
    'h',  'i',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 'j',  'k',  'l',  'm',  'n',  'o',  'p',   // 90
    'q',  'r',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 's',  't',  'u',  'v',  'w',  'x',      /* A0 - BF */
    'y',  'z',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, '\\', '{',  '}',  '[',  ']',  0x00, 0x00,   //b0
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 'A',  'B',  'C',  'D',  'E',  'F',  'G',      /* C0 - DF */
    'H',  'I',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 'J',  'K',  'L',  'M',  'N',  'O',  'P',    //d0
    'Q',  'R',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 'S',  'T',  'U',  'V',  'W',  'X',      /* E0 - FF */
    'Y',  'Z',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  // f0
    '8',  '9',  0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,
};

unsigned int parity7[64] = {
    0100, 0000, 0000, 0100, 0000, 0100, 0100, 0000    /* 00 - 07 */
    ,0000, 0100, 0100, 0000, 0100, 0000, 0000, 0100    /* 10 - 17 */
    ,0000, 0100, 0100, 0000, 0100, 0000, 0000, 0100    /* 20 - 27 */
    ,0100, 0000, 0000, 0100, 0000, 0100, 0100, 0000    /* 30 - 37 */
    ,0000, 0100, 0100, 0000, 0100, 0000, 0000, 0100    /* 40 - 47 */
    ,0100, 0000, 0000, 0100, 0000, 0100, 0100, 0000    /* 50 - 57 */
    ,0100, 0000, 0000, 0100, 0000, 0100, 0100, 0000    /* 60 - 67 */
    ,0000, 0100, 0100, 0000, 0100, 0000, 0000, 0100};  /* 70 - 77 */


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
    '_',    /* 47  B 8421  - delta */
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




FILE *infile, *outfile;
int  upkx;

void usage() {
    fprintf(stderr,"Usage:  sds_9pal  [-l] [-x nnnn-tt] <tapefile> <outfile> [>stdout]\n");
    fprintf(stderr,"    -l :  List all catalog numbers \n");
    fprintf(stderr,"    -x nnnnn-tt : Extract catalog number nnnnn ");
    fprintf(stderr,"element type tt\n");
    fprintf(stderr,"       if type = 34 file, extracted ascii data is ");
    fprintf(stderr,"written to stdout\n");
    exit(1);
}


/* Returns the BCD of the hollerith code or 0x7f if error */
uint8_t
sim_hol_to_bcd(uint16_t hol) {
    uint8_t                bcd;
    
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





//  get int32_t value from input file
uint32_t getint() {
    char tmp[4];
    uint32_t val;
    for (int j = 3; j>= 0;j--) {
        fread(&tmp[j],1,1,infile);
    }
    val = *(uint32_t *)&tmp[0];
    return val;
}

// return 4 6 bit bytes packed into a word
int getwd(int icnt)  {
    static int res;                     // unpack 3-4 index
    static char upkchr;                // unpack temp byte
    static int  upkbytctr;             // unpack byte counter
    int ch;
    
    //for (int i = 0; i < 4;i++) {
    while(icnt) {
        switch (upkx) {
            case 0:
                fread(&upkchr, sizeof(unsigned char), 1, infile);
                upkbytctr++;
                icnt--;                   // used first char
                ch = (upkchr >> 2) & 077;
                upkx++;
                res = 0;
                break;
            case 1:
                ch = (upkchr & 03) << 4;
                fread(&upkchr, sizeof(unsigned char), 1, infile);
                upkbytctr++;
                icnt--;                   // used first and second char
                ch = ch | ((upkchr >> 4) & 017);
                upkx++;
                break;
            case 2:
                ch = ((upkchr & 017) << 2);
                fread(&upkchr, sizeof(unsigned char), 1, infile);
                upkbytctr++;
                ch = ch | ((upkchr >> 6) & 03);  // used second and third char
                upkx++;
                break;
            case 3:
                ch = upkchr & 077;
                icnt--;                        // used third character again
                upkx = 0;
                break;
        }
        res = res<<6 | ch;
    }
    return res;
}

// read simh length word
int rdlen() {
    char xlen[4];
    int sz;
    
    if ((sz = fread(xlen, sizeof(unsigned char), 4, infile)) != 4)
        return 0;
   
    /* Convert to number */
    sz = (xlen[0] & 0xff);
    sz |= ((xlen[1])  & 0xff)<< 8;
    sz |= ((xlen[2])  & 0xff)<< 16;
    sz |= ((xlen[3]) & 0xff) << 24;
    /* Check for EOM */
    if (sz == 0xffffffff)  {
        return -2;
    }
    /* Check for EOF */
    if (sz == 0) {
        return -1;
    }
    return sz;
}

//  Read the label at the beginning of the tape

int getbot(void) {
    int sz;
    char lbl[60];
    int i;
    
    sz = rdlen();
    if (sz != 12) {
        fprintf(stderr,":LBL size error\n");
        return 0;
    }
    if (fread(&lbl,1,sz, infile) != sz)  {
        fprintf(stderr,":LBL read error\n");
        return 0;
    }
    if ((ebcdic_to_ascii[lbl[0]&0xff] != ':') ||
        (ebcdic_to_ascii[lbl[1]&0xff] != 'L') ||
        (ebcdic_to_ascii[lbl[2]&0xff] != 'B') ||
        (ebcdic_to_ascii[lbl[3]&0xff] != 'L')) {
        fprintf(stderr,":LBL missing\n");
        return 0;
    }
    fread(&lbl, sizeof(unsigned char), 4, infile);
    
    sz = rdlen();
    if (sz != 28) {
        fprintf(stderr,":ACN size error\n");
        return 0;
    }
   if (fread(&lbl,1,sz, infile) != sz)  {
        fprintf(stderr,":ACN read error\n");
        return 0;
    }
    if ((ebcdic_to_ascii[lbl[0]&0xff] != ':') ||
        (ebcdic_to_ascii[lbl[1]&0xff] != 'A') ||
        (ebcdic_to_ascii[lbl[2]&0xff] != 'C') ||
        (ebcdic_to_ascii[lbl[3]&0xff] != 'N')) {
        fprintf(stderr,":ACN missing\n");
        return 0;
    }
    fread(&lbl, sizeof(unsigned char), 4, infile);
    
    // should be an EOF here
    sz = rdlen();
    if (sz != -1) {
        fprintf(stderr,"Missing EOF following ACN\n");
        return 0;
    }
    return sz;
}


int getbof(char *buf) {
    int sz;
    char lbl[60];
    int i;
    
    sz = rdlen();
    if (sz != 52) {
        if (sz == 12) {
            if (fread(&lbl,1,sz, infile) != sz)  {
                fprintf(stderr,":BOF read error\n");
                return 0;
            }
            if ((ebcdic_to_ascii[lbl[0]&0xff] != ':') ||
                (ebcdic_to_ascii[lbl[1]&0xff] != 'E') ||
                (ebcdic_to_ascii[lbl[2]&0xff] != 'O') ||
                (ebcdic_to_ascii[lbl[3]&0xff] != 'R')) {
                fprintf(stderr,":BOF/EOR missing end of tape?\n");
                return 0;
            }
            fread(&lbl, sizeof(unsigned char), 4, infile);
            //fprintf(stderr,":EOR at end of tape\n");
            return 0;
        }
            
        fprintf(stderr,":BOF size error %d %x\n",sz,sz);
        //return 0;
    }
    if (fread(buf,1,sz, infile) != sz)  {
        fprintf(stderr,":BOF read error\n");
        return 0;
    }
    if ((ebcdic_to_ascii[buf[0]&0xff] != ':') ||
        (ebcdic_to_ascii[buf[1]&0xff] != 'B') ||
        (ebcdic_to_ascii[buf[2]&0xff] != 'O') ||
        (ebcdic_to_ascii[buf[3]&0xff] != 'F')) {
        fprintf(stderr,":BOF missing %x %x %x %x\n",buf[0]&0xff,buf[1]&0xff,buf[2]&0xff,buf[3]&0xff);
        return 0;
    }
    fread(&lbl, sizeof(unsigned char), 4, infile);
    // should be an EOF here
    if (rdlen() != -1) {
        fprintf(stderr,"Missing EOF following BOF\n");
        return 0;
    }
    return sz;
}
    
int geteof(void) {
    int sz;
    char lbl[60];
    int i;
    
    sz = rdlen();
    if (sz != 12) {
        fprintf(stderr,":EOF size error\n");
        return 0;
    }
    if (fread(&lbl,1,sz, infile) != sz)  {
        fprintf(stderr,":EOF read error\n");
        return 0;
    }
    if ((ebcdic_to_ascii[lbl[0]&0xff] != ':') ||
        (ebcdic_to_ascii[lbl[1]&0xff] != 'E') ||
        (ebcdic_to_ascii[lbl[2]&0xff] != 'O') ||
        (ebcdic_to_ascii[lbl[3]&0xff] != 'F')) {
        fprintf(stderr,":EOF missing\n");
        return 0;
    }
    fread(&lbl, sizeof(unsigned char), 4, infile);
    // should be an EOF here
    if (rdlen() != -1) {
        fprintf(stderr,"Missing EOF following :EOF\n");
        return 0;
    }
    return sz;
}


//  Extract data
//  output card images suitable for sim_card reader and sds_cr
void extract(int pnum,int ptype) {
    
    int32_t blksz;              // leading length in simh block
    int32_t finsz;              // final length in simh block
    int32_t remsz;
    int32_t bytcnt;             // counter of input bytes from block
    int32_t length;             // number bytes in record data to unpack
    int      mod;                // number bytes of incomplete unpack dat
    int     mrkflg;
    unsigned char tmp;
    int    j;
    int32_t data;
    int32_t numrec;             // number records in block
    int32_t totrec;             // total number records in deck
    int32_t reccnt;             // counter of records in block
    int32_t recnum = -1;             // record sequence number
    

    totrec = 0;
    mod = 0;
    // read blocks to end of file marker
    while ((blksz = rdlen()) > 0) {          // leading length
        reccnt = 0;
        numrec = getint();                  // record count
        bytcnt = 4;
        //printf("numrec  0x%x \n",numrec);
        // unpack records
        while (bytcnt < blksz) {
            remsz = blksz - bytcnt;            
            data = getint() & 0xffffff;    // record number
            bytcnt += 4;
            if (data == recnum) {            // continued record
                totrec--;
                mrkflg = 0;                 // no record mark
            }
            else {
                if (ptype == 34)
                    printf("\n");
                mrkflg = 1;
            }
            recnum = data;
            
            length = getint() & 0xffffff;    // length of record data
            bytcnt += 4;
            if (mod) {
                data = getwd(3-mod);        // unpack remainder of word
                if (ptype == 34) {
                    printf("%c",bcd_ascii[sim_hol_to_bcd(data>>12 &0xfff)]);
                    printf("%c",bcd_ascii[sim_hol_to_bcd(data &0xfff)]);
                }
                bytcnt += 3-mod;
                for (int i = 18; i >= 0;i-=6) {
                    tmp = ((data >> i ) & 0x3f);
                    tmp = tmp | parity7[tmp];
                    fwrite(&tmp,1,1,outfile);

                }
                length -= (3-mod);
                mod = 0;
            }
            mod = length % 3;
            for (j = 0; j < length - mod; j+=3,bytcnt+=3) {
                upkx = 0;
                data = getwd(3);           // output full words
                 if (ptype == 34) {
                    printf("%c",bcd_ascii[sim_hol_to_bcd(data>>12 &0xfff)]);
                    printf("%c",bcd_ascii[sim_hol_to_bcd(data &0xfff)]);
                }
                for (int i = 18; i >= 0;i-=6) {
                    tmp = ((data >> i ) & 0x3f);
                    tmp = tmp | parity7[tmp];
                    if ((j == 0) & (i == 18) & mrkflg)
                        tmp |= 0x80;        // Set record mark
                    fwrite(&tmp,1,1,outfile);
                }
            }
            if (mod) {
                tmp = getwd(mod);           // unpack partial word
                bytcnt+=mod;
            }
            reccnt++;
            totrec++;
        }
        finsz = rdlen();
        if (finsz != blksz)
            printf(" end length not equal  %x  %x\n",blksz,finsz);
    }
    fflush(stdout);
    fprintf(stderr,"\n\ncatalog number %d-%d  %d cards\n",pnum,ptype,recnum+1);
    return;
}



int main(int argc, char *argv[]) {
    int lst = 0;
    int ext = 0;
    int cat = 0;
    int elm = 0;
    char bof[120];
    char asc[120];
    char tmp;
    int pnum;
    int ptype;
    int sz;
    int blksz;
    
    if ((argc < 2) || (argc > 5)){
        usage();
        return 0;
    }
    while(--argc && **(++argv) == '-') {
        switch(tolower((*argv)[1])) {
            case 'l':
                lst = 1;
                break;
            case 'x':
                if (sscanf(argv[1],"%d-%d",&cat,&elm) != 2)
                    usage();
                if ((elm != 24) && (elm != 34) &&
                    (elm != 44) && (elm != 84)) {
                    fprintf(stderr,"Unsupported element type\n");
                    return 0;
                }
                ext = 1;
                --argc;
                ++argv;
                break;
                
            default:
                usage();
                return 0;
        }
    }

    infile = fopen(argv[0], "rb");
    if (!infile) {
        printf("Cannot open PAL file: %s\n", argv[0]);
        return 1;
    }
    outfile = fopen(argv[1], "w");
    if (!outfile) {
        printf("Cannot open output file: %s\n", argv[1]);
        return 1;
    }
    
    getbot();
    while (1) {
        sz = getbof(bof);
        if (sz == 0)
            break;              // end of input tape
        
        for (int i = 0; i < sz; i++) {
            asc[i] = ebcdic_to_ascii[bof[i]&0xff];
        }
        sscanf(&asc[9],"%6d-%2d",&pnum,&ptype);
        
        if (lst)
            fprintf(outfile,"catalog number %d-%d\n",pnum,ptype);
        else if (ext && (pnum == cat) && (ptype == elm)) {
            extract(pnum,ptype);
            return 0;
        }
        while ((blksz = rdlen()) != -1) {
            if (feof(infile) != 0) {
                printf("Unexpected EOF on infile\n");
                return 0;
            }
            for (int j = 0; j < blksz; j++) {
                fread(&tmp,1,1,infile);
            }
            rdlen();
        }
        geteof();
    }
    if (ext)
        fprintf(stderr,"Missing file: %d-%d\n",cat,elm);
}
