/* sds_listtape.c - list contents of SDS simh tape*/

/*
 This is a copy of listtape, created by Rich Cornwell.
 
 It is modified slightly to recognize SDS 060 character as blank
 and to ignore erase gap code at the beginning of file.
 Modified by Ken Rector, Feb. 12, 2020.
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <strings.h>
#include <ctype.h>

#define TAPE_BUFFER_SIZE 100000L


#define TAPE_IRG 0200
#define BCD_TM 017
                      
char		buffer[TAPE_BUFFER_SIZE];
char		*xlat;		/* Pointer to translate table */
int		eor = 0;	/* Report eor */
int		bin = 0;	/* Doing binary */
int		p7b = 0;	/* Doing BCD tape */
int		mark = 0;	/* Show marks */
int		com = 0;	/* Show as commercial */
int		cc = 0;		/* Process print control chars */
int		ibm029 = 0;	/* Translate using IBM 029 codes */
int		auto_bcd = 0;	/* Automatic translation of binary */
int		reclen = 130;
int		dis = 0;	/* Display code */
int		cosy = 0;	/* Expand COSY blanks */
int		bci = 0;	/* Burroughs BCI code */
int		univac = 0;	/* Univac code. */

int bytecnt = 0;


char parity_table[64] = {
        /* 0    1    2    3    4    5    6    7 */
        0000,0100,0100,0000,0100,0000,0000,0100,
        0100,0000,0000,0100,0000,0100,0100,0000,
        0100,0000,0000,0100,0000,0100,0100,0000,
        0000,0100,0100,0000,0100,0000,0000,0100,
        0100,0000,0000,0100,0000,0100,0100,0000,
        0000,0100,0100,0000,0100,0000,0000,0100,
        0000,0100,0100,0000,0100,0000,0000,0100,
        0100,0000,0000,0100,0000,0100,0100,0000
};

char bcd_ascii[64] = {
	'_',	/* 0           - space */
	'1',	/* 1        1  - 1 */
	'2',	/* 2       2   - 2 */
	'3',	/* 3       21  - 3 */
	'4',	/* 4      4    - 4 */
	'5',	/* 5      4 1  - 5 */
	'6',    /* 6      42   - 6 */
	'7',	/* 7	  421  - 7 */
	'8',	/* 8     8     - 8 */
	'9',	/* 9     8  1  - 9 */
	'0',	/* 10    8 2   - 0 */
	'=',    /* 11    8 21  - equal */
	'\'',	/* 12    84    - apostrophe */
	':',    /* 13    84 1  - colon */
	'>',	/* 14    842   - greater than */
	'"',	/* 15    8421  - radical 017 {? */
	' ',    /* 16   A      - substitute blank */
	'/',	/* 17   A   1  - slash */
	'S',	/* 18   A  2   - S */
	'T',	/* 19   A  21  - T */
	'U',	/* 20   A 4    - U */
	'V',	/* 21   A 4 1  - V */
	'W',	/* 22   A 42   - W */
	'X',	/* 23   A 421  - X */
	'Y',	/* 24   A8     - Y */
	'Z',	/* 25   A8  1  - Z */
	'#',	/* 26   A8 2   - record mark */
	',',	/* 27   A8 21  - comma */
	'(',	/* 28   A84    - paren */
	'`',	/* 29   A84 1  - word separator */
	'\\',	/* 30   A842   - left oblique */
	'{',    /* 31   A8421  - segment mark */
	'-',	/* 32  B       - hyphen */
	'J',	/* 33  B    1  - J */
	'K',	/* 34  B   2   - K */
	'L',	/* 35  B   21  - L */
	'M',	/* 36  B  4    - M */
	'N',	/* 37  B  4 1  - N */
	'O',	/* 38  B  42   - O */
	'P',	/* 39  B  421  - P */
	'Q',	/* 40  B 8     - Q */
	'R',	/* 41  B 8  1  - R */
	'!',	/* 42  B 8 2   - exclamation */
	'$',	/* 43  B 8 21  - dollar sign */
	'*',	/* 44  B 84    - asterisk */
	']',	/* 45  B 84 1  - right bracket */
	';',    /* 46  B 842   - semicolon */
	'_',    /* 47  B 8421  - delta */
	'+',    /* 48  BA      - ampersand or plus */
	'A',	/* 49  BA   1  - A */
	'B',    /* 50  BA  2   - B */
	'C',	/* 51  BA  21  - C */
	'D',	/* 52  BA 4    - D */
	'E',	/* 53  BA 4 1  - E */
	'F',	/* 54  BA 42   - F */
	'G',	/* 55  BA 421  - G */
	'H',	/* 56  BA8     - H */
	'I',	/* 57  BA8  1  - I */
	'?',	/* 58  BA8 2   - question mark 032 */
	'.',	/* 59  BA8 21  - period */
	')',	/* 60  BA84    - paren */
	'[',	/* 61  BA84 1  - left bracket 035 */
	'<',	/* 62  BA842   - less than 036 */
	'}'	/* 63  BA8421  - group mark 037 */
};

char bcd029_ascii[64] = {
        '#',    /* 0           - space */
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
        '@',    /* 11    8 21  - equal */
        '?',    /* 12    84    - apostrophe */
        ':',    /* 13    84 1  - colon */
        '>',    /* 14    842   - greater than */
        '}',    /* 15    8421  - radical 017 {? */
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
        ',',    /* 26   A8 2   - record mark */
        '%',    /* 27   A8 21  - comma */
        '!',    /* 28   A84    - paren */
        '=',    /* 29   A84 1  - word separator */
        ']',    /* 30   A842   - left oblique */
        '"',    /* 31   A8421  - segment mark */
        '|',    /* 32  B       - hyphen */
        'J',    /* 33  B    1  - J */
        'K',    /* 34  B   2   - K */
        'L',    /* 35  B   21  - L */
        'M',    /* 36  B  4    - M */
        'N',    /* 37  B  4 1  - N */
        'O',    /* 38  B  42   - O */
        'P',    /* 39  B  421  - P */
        'Q',    /* 40  B 8     - Q */
        'R',    /* 41  B 8  1  - R */
        '$',    /* 42  B 8 2   - exclamation */
        '*',    /* 43  B 8 21  - dollar sign */
        '-',    /* 44  B 84    - asterisk */
        ')',    /* 45  B 84 1  - right bracket */
        ';',    /* 46  B 842   - semicolon */
        '{',    /* 47  B 8421  - delta */
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
        '.',    /* 58  BA8 2   - question mark 032 */
        '[',    /* 59  BA8 21  - period */
        '&',    /* 60  BA84    - paren */
        '(',    /* 61  BA84 1  - left bracket 035 */
        '<',    /* 62  BA842   - less than 036 */
        '~'     /* 63  BA8421  - group mark 037 */
};


char bci_ascii[64] = {
	'0',	/* 0           - space */
	'1',	/* 1        1  - 1 */
	'2',	/* 2       2   - 2 */
	'3',	/* 3       21  - 3 */
	'4',	/* 4      4    - 4 */
	'5',	/* 5      4 1  - 5 */
	'6',    /* 6      42   - 6 */
	'7',	/* 7	  421  - 7 */
	'8',	/* 8     8     - 8 */
	'9',	/* 9     8  1  - 9 */
	'#',	/* 10    8 2   - 0 */
	'@',    /* 11    8 21  - atsign */
	'?',	/* 12    84    - question */
	':',    /* 13    84 1  - colon */
	'>',	/* 14    842   - greater than */
	'}',	/* 15    8421  - greater or equal */
	'+',    /* 48  BA      - ampersand or plus */
	'A',	/* 49  BA   1  - A */
	'B',    /* 50  BA  2   - B */
	'C',	/* 51  BA  21  - C */
	'D',	/* 52  BA 4    - D */
	'E',	/* 53  BA 4 1  - E */
	'F',	/* 54  BA 42   - F */
	'G',	/* 55  BA 421  - G */
	'H',	/* 56  BA8     - H */
	'I',	/* 57  BA8  1  - I */
	'.',	/* 58  BA8 2   - period */
	'[',	/* 59  BA8 21  - left bracket */
	'&',	/* 60  BA84    - and */
	'(',	/* 61  BA84 1  - left paren 035 */
	'<',	/* 62  BA842   - less than 036 */
	'~',	/* 63  BA8421  - left arrow 037 */
	'|',	/* 32  B       - bar */
	'J',	/* 33  B    1  - J */
	'K',	/* 34  B   2   - K */
	'L',	/* 35  B   21  - L */
	'M',	/* 36  B  4    - M */
	'N',	/* 37  B  4 1  - N */
	'O',	/* 38  B  42   - O */
	'P',	/* 39  B  421  - P */
	'Q',	/* 40  B 8     - Q */
	'R',	/* 41  B 8  1  - R */
	'$',	/* 42  B 8 2   - dollar sign */
	'*',	/* 43  B 8 21  - asterisk */
	'-',	/* 44  B 84    - minus */
	')',	/* 45  B 84 1  - right paren */
	';',    /* 46  B 842   - semicolon */
	'{',    /* 47  B 8421  - less or equal */
	' ',    /* 16   A      - substitute blank */
	'/',	/* 17   A   1  - slash */
	'S',	/* 18   A  2   - S */
	'T',	/* 19   A  21  - T */
	'U',	/* 20   A 4    - U */
	'V',	/* 21   A 4 1  - V */
	'W',	/* 22   A 42   - W */
	'X',	/* 23   A 421  - X */
	'Y',	/* 24   A8     - Y */
	'Z',	/* 25   A8  1  - Z */
	',',	/* 26   A8 2   - comma */
	'%',	/* 27   A8 21  - percent */
	'!',	/* 28   A84    - not equal */
	'=',	/* 29   A84 1  - equal */
	']',	/* 30   A842   - right brack */
	'"'	/* 31   A8421  - qoute */
};


char dis_ascii[64] = {
	':',	/* 0           - space */
	'A',	/* 1        1  - 1 */
	'B',	/* 2       2   - 2 */
	'C',	/* 3       21  - 3 */
	'D',	/* 4      4    - 4 */
	'E',	/* 5      4 1  - 5 */
	'F',    /* 6      42   - 6 */
	'G',	/* 7	  421  - 7 */
	'H',	/* 8     8     - 8 */
	'I',	/* 9     8  1  - 9 */
	'J',	/* 10    8 2   - 0 */
	'K',    /* 11    8 21  - equal */
	'L',	/* 12    84    - apostrophe */
	'M',    /* 13    84 1  - colon */
	'N',	/* 14    842   - greater than */
	'O',	/* 15    8421  - radical 017 {? */
	'P',    /* 16   A      - substitute blank */
	'Q',	/* 17   A   1  - slash */
	'R',	/* 18   A  2   - S */
	'S',	/* 19   A  21  - T */
	'T',	/* 20   A 4    - U */
	'U',	/* 21   A 4 1  - V */
	'V',	/* 22   A 42   - W */
	'W',	/* 23   A 421  - X */
	'X',	/* 24   A8     - Y */
	'Y',	/* 25   A8  1  - Z */
	'Z',	/* 26   A8 2   - record mark */
	'0',	/* 27   A8 21  - comma */
	'1',	/* 28   A84    - paren */
	'2',	/* 29   A84 1  - word separator */
	'3',	/* 30   A842   - left oblique */
	'4',    /* 31   A8421  - segment mark */
	'5',	/* 32  B       - hyphen */
	'6',	/* 33  B    1  - J */
	'7',	/* 34  B   2   - K */
	'8',	/* 35  B   21  - L */
	'9',	/* 36  B  4    - M */
	'+',	/* 37  B  4 1  - N */
	'-',	/* 38  B  42   - O */
	'*',	/* 39  B  421  - P */
	'/',	/* 40  B 8     - Q */
	'(',	/* 41  B 8  1  - R */
	')',	/* 42  B 8 2   - exclamation */
	'$',	/* 43  B 8 21  - dollar sign */
	'=',	/* 44  B 84    - asterisk */
	' ',	/* 45  B 84 1  - right bracket */
	',',    /* 46  B 842   - semicolon */
	'.',    /* 47  B 8421  - delta */
	'\'',    /* 48  BA      - ampersand or plus */
	'[',	/* 49  BA   1  - A */
	']',    /* 50  BA  2   - B */
	'%',	/* 51  BA  21  - C */
	'!',	/* 52  BA 4    - D */
	'a',	/* 53  BA 4 1  - E */
	'b',	/* 54  BA 42   - F */
	'c',	/* 55  BA 421  - G */
	'd',	/* 56  BA8     - H */
	'e',	/* 57  BA8  1  - I */
	'f',	/* 58  BA8 2   - question mark 032 */
	'g',	/* 59  BA8 21  - period */
	'h',	/* 60  BA84    - paren */
	'i',	/* 61  BA84 1  - left bracket 035 */
	'j',	/* 62  BA842   - less than 036 */
	';'	/* 63  BA8421  - group mark 037 */
};

char univ_ascii[64] = {
        '@',    /* 0           - space */
        '[',    /* 1        1  - 1 */
        ']',    /* 2       2   - 2 */
        '#',    /* 3       21  - 3 */
        '_',    /* 4      4    - 4 */
        ' ',    /* 5      4 1  - 5 */
        'A',    /* 49  BA   1  - A */
        'B',    /* 50  BA  2   - B */
        'C',    /* 51  BA  21  - C */
        'D',    /* 52  BA 4    - D */
        'E',    /* 53  BA 4 1  - E */
        'F',    /* 54  BA 42   - F */
        'G',    /* 55  BA 421  - G */
        'H',    /* 56  BA8     - H */
        'I',    /* 57  BA8  1  - I */
        'J',    /* 33  B    1  - J */
        'K',    /* 34  B   2   - K */
        'L',    /* 35  B   21  - L */
        'M',    /* 36  B  4    - M */
        'N',    /* 37  B  4 1  - N */
        'O',    /* 38  B  42   - O */
        'P',    /* 39  B  421  - P */
        'Q',    /* 40  B 8     - Q */
        'R',    /* 41  B 8  1  - R */
        'S',    /* 18   A  2   - S */
        'T',    /* 19   A  21  - T */
        'U',    /* 20   A 4    - U */
        'V',    /* 21   A 4 1  - V */
        'W',    /* 22   A 42   - W */
        'X',    /* 23   A 421  - X */
        'Y',    /* 24   A8     - Y */
        'Z',    /* 25   A8  1  - Z */
        ')',    /* 6      42   - 6 */
        '-',    /* 7      421  - 7 */
        '+',    /* 8     8     - 8 */
        '<',    /* 9     8  1  - 9 */
        '=',    /* 10    8 2   - 0 */
        '>',    /* 11    8 21  - equal */
        '&',    /* 12    84    - apostrophe */
        '$',    /* 13    84 1  - colon */
        '*',    /* 14    842   - greater than */
        '(',    /* 15    8421  - radical 017 {? */
        '%',    /* 16   A      - substitute blank */
        ':',    /* 17   A   1  - slash */
        '?',    /* 26   A8 2   - record mark */
        '!',    /* 27   A8 21  - comma */
        '\'',   /* 28   A84    - paren */
        '\\',   /* 29   A84 1  - word separator */
        '0',    /* 30   A842   - left oblique */
        '1',    /* 31   A8421  - segment mark */
        '2',    /* 32  B       - hyphen */
        '3',    /* 42  B 8 2   - exclamation */
        '4',    /* 43  B 8 21  - dollar sign */
        '5',    /* 44  B 84    - asterisk */
        '6',    /* 45  B 84 1  - right bracket */
        '7',    /* 46  B 842   - semicolon */
        '8',    /* 47  B 8421  - delta */
        '9',    /* 48  BA      - ampersand or plus */
        '`',    /* 58  BA8 2   - question mark 032 */
        ';',    /* 59  BA8 21  - period */
        '/',    /* 60  BA84    - paren */
        '.',    /* 61  BA84 1  - left bracket 035 */
        '=',    /* 62  BA842   - less than 036 */
        '!'     /* 63  BA8421  - group mark 037 */
};


void usage() {
    fprintf(stderr,"SDS listtape\n");
    fprintf(stderr,"     Use no options, simply type \"sds_listtape <tapefile>\"\n");
    
   fprintf(stderr,"Usage: listtape [-b] [-e] [-p] [-r#] <tapefile>\n");
   fprintf(stderr,"     -r#: Characters per record #\n");
   fprintf(stderr,"     -a:  Auto Binary/BCD translation\n");
   fprintf(stderr,"     -b:  Use IBSYS binary translation\n");
   fprintf(stderr,"     -d:  Use CDC Display Code translation\n");
   fprintf(stderr,"     -9:  Use IBM029 translation\n");
   fprintf(stderr,"     -m:  Show record marks |\n");
   fprintf(stderr,"     -e:  Show end of records as {\n");
   fprintf(stderr,"     -p:  Read BCD tape instead of TAP format\n");
   fprintf(stderr,"     -c:  Print with commerical charset\n");
   fprintf(stderr,"     -l:  Process listing control chars\n");
   fprintf(stderr,"     -i:  Display BCI character encoding\n");
   fprintf(stderr,"     -u:  Use univac character encoding\n");
   fprintf(stderr,"     -z:  Display CDC Cosy records\n");
   exit(1);
}

/* Read one record from tape */
int read_tape(FILE *f, int *len) {
   unsigned long int sz;
   *len = 0;
   if (p7b) {
	static unsigned char lastchar = 0xff;
	unsigned char	    ch;
        sz = 0;
	if (lastchar != 0xff) 
	    buffer[sz++] = lastchar;
	/* Check if last char was Tape Mark */
	else if (lastchar == (BCD_TM|TAPE_IRG)) {
	    lastchar = 0xff;
	    *len = -1;
	    return 1;
	}
	lastchar = 0xff;
	while(fread(&ch, sizeof(unsigned char), 1, f) == 1) {
	    if (ch & TAPE_IRG) {
	        lastchar = ch;
		*len = sz;
	        return 1;
	    }
	    buffer[sz++] = ch;
	}
	if (sz != 0) {
	   *len = sz;
	   return 1;
	}
	return 0;
   } else {
       unsigned char	xlen[4];
       int		i;
       if (fread(&xlen, sizeof(unsigned char), 4, f) != 4)
           return 0;
       bytecnt += 4;
       
       /* Convert to number */
       sz = xlen[0];
       sz |= (xlen[1]) << 8;
       sz |= (xlen[2]) << 16;
       sz |= (xlen[3]) << 24;
       
       /* skip over Erase Gap codes */
       while (sz == -2) {
           if (fread(&xlen, sizeof(unsigned char), 4, f) != 4)
               return 0;
            bytecnt += 4;
           
           /* Convert to number */
           sz = xlen[0];
           sz |= (xlen[1]) << 8;
           sz |= (xlen[2]) << 16;
           sz |= (xlen[3]) << 24;
       }
       
       /* Check for EOM */
       if (sz == 0xffffffff)  {
           *len = -2;
           return 0;
       }
       /* Check for EOF */
       if (sz == 0) {
           *len = -1;
           return 1;
       }
       if (sz > TAPE_BUFFER_SIZE) {
           fprintf(stderr, "Block to big for buffer @%x %lx\n",bytecnt,sz);
           return 0;
       }
       *len = sz;
       sz = 0x7fffffff & ((sz + 1) & ~1);
       if (fread(buffer, 1, sz, f) != sz)  {
           fprintf(stderr, "read error\n");
           return 0;
       }
       bytecnt += sz;
       /* Read backward length */
       fread(&sz, sizeof(unsigned char), 4, f);
        bytecnt += 4;
   }
   return 1;
}



int main(int argc, char *argv[]) {
   int		sz;
   int		i;
   int		col;
   char		*p;
   FILE		*tape;
   int		cosy_rec;

   xlat = &bcd_ascii[0];
   while(--argc && **(++argv) == '-') {
   	switch(tolower((*argv)[1])) {
	case 'r':
		reclen = atoi(&(*argv)[2]);
      	fprintf(stderr,"Recordlen set: %d\n",reclen);
		break;
	case 'a':
		auto_bcd = 1;
		break;
	case 'e':
		eor = 1;
		break;
	case 'd':
		dis = 1;
		xlat = &dis_ascii[0];
		break;
	case 'b':
		bin = 1;
		break;
	case 'p':
		p7b = 1;
		break;
	case 'm':
		mark = 1;
		break;
	case 'c':
		com = 1;
		break;
 	case 'l':
		cc = 1;
		break;
	case 'u':
		univac = 1;
		xlat = &univ_ascii[0];
		break;
	case 'i':
		bci = 1;
		xlat = &bci_ascii[0];
		break;
	case '9':
		ibm029 = 1;
		xlat = &bcd029_ascii[0];
		break;
	case 'z':
	        cosy = 1;
		break;
      default:
      	fprintf(stderr,"Unknown option: %s\n",*argv);
      }
   }

   if(argc != 1) {
   	usage();
   }

   /* Open input */
   if((tape = fopen(*argv,"rb")) == NULL) {
	fprintf(stderr,"Can't open tape file %s: ",*argv);
        perror("");
        exit(1);
   }

    bytecnt = 0;
    
   /* Process records of the file */
   while(read_tape(tape, &sz)) {
        cosy_rec = 0;
	if (sz == -2) 
	    break;
	if (sz == -1) 
	    puts("*EOF*");
	else {
	    p = buffer;
	    col = 0;
	    for(i = 0; i < sz; i++)  {
		char	ch = *p++;
		if (auto_bcd) {
		    if (parity_table[ch & 077] == (ch & 0100)) 
			bin = 0;
		    else
			bin = 1;
		}
                ch &= 077;
		if (bin) {
		    ch ^= (ch & 020) << 1;
		    if (ch == 012)
		        ch = 0;   
		    else if (ch == 0)
			ch = 012;
		}
		if (ch == 032 && !(dis | univac | bci)) {
		   if (mark) {
			putchar(xlat[ch]);
			col++;
		   } else {
			putchar('\n');
			col = 0;
		   }
		} else {
		    int asc = xlat[ch];
		    if (cosy) {
		    	if (ch == 0) 
			    asc = '\n';
		    	else if (ch >= 064) {
			    int bl = ch - 062;
			    asc = ' ';
			    if (ch >= 074) 
			    	bl = 10 * (ch - 073);
			    while(--bl > 0) {
			    	putchar(' ');
			    	col++;
			    }
			}
		    } 
		    if (com) {
		        switch(asc) {
		        case '+': asc = '&'; break;
			case '(': asc = '%'; break;
			case '\'': asc = '@'; break;
		        }
	 	   }
		   if (cc && col == 0) {
			switch(asc) {
			case '1': putchar('\f'); break;
			case '2': putchar('\n');
			case '0': putchar('\n'); 
			default: break;
			}
		   } else 
		       putchar(asc);
		   if((++col == reclen && sz != (i+1))){
		       putchar('\n');
		       col = 0;
		   }
		}
	    }
	    if (eor)
		putchar('{');
	    putchar('\n');
	}
    }
    fclose(tape);
}
