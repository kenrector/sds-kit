/***********************************************************************
*
* Build up a deck. Remove blank and invalid cards.
*
***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int	sequence = 0;
int	cardno = 1;
int	image_mode = 0;
int	fcard = 0;
int	lcard = -1;
int	ccard = 0;
char	*label = NULL;
int	wrdnum;

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


/* Dump card image to output file */
void dump_card(FILE *of, unsigned short image[80]) {
    int 	   t;
    int		   i, j;
    unsigned char  out[160]; 


    /* Fill in label columns */
    if (label) {
	char	*p = label;
        unsigned short	temp;
	for (i = 72; i < 80; i++) {
	    temp = ascii_to_hol[*p++];
	    if ((temp & 0xf000) == 0) 
	         image[i] = temp;
	    
	}
    }

    /* Add sequence */
    if (sequence) {
	 i = cardno;
	 j = 79;
	 while( i != 0) {
	    t = i % 10;
	    if (t == 0) 
		image[j] = 0x200;
	    else
		image[j] = 1 << (9 - t);
	    j--;
	    i -= t;
	    i /= 10;
	 }
    }

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
    fwrite(out, 1, i, of);
    cardno++;
}

/* Dump a text file as hollerith based images */
void dump_text(FILE *of, FILE *f) {
    int		        col;
    unsigned short	image[80];
    char		ch;
    unsigned short	temp;

    memset(image, 0, sizeof(image));	/* Clear image */
    col = 0;
    while(!feof(f)) {
	ch = fgetc(f);
	switch(ch) {
	case '\0':
	case '\r':	break;
	default:
		temp = ascii_to_hol[ch];
		if ((temp & 0xf000) == 0 && col < 80) 
		    image[col++] = temp;
		break;
	case '\n':
		dump_card(of, image);
                memset(image, 0, sizeof(image));
                col = 0;
		break;
	case '\t':
		col = (col | 7) + 1; /* Mult of 8 */
		break;
	}
     }
     if (col != 0)
         dump_card(of, image);
}

/* Produce a transfer card */
void transfer_card(char *addr, FILE *of) {
    unsigned short image[80];
    int		loc = 0;
    int		i;

    memset(image, 0, sizeof(image));
    image[43] = 1;
    if (*addr == '0') {	/* Octal */
	while(*addr != '\0') {
	    loc <<= 3;
	    loc += *addr++ - '0';
	}
    } else {	/* Decimal */
	while(*addr != '\0') {
	    loc *= 10;
	    loc += *addr++ - '0';
	}
    }
    for(i = 71; i > 56; i--) {
	if (loc & 1)
	   image[i] = 1;
	loc >>= 1;
    }
    dump_card(of, image);
}

/* Set address for correction card */
void set_addr(char *addr, unsigned short image[80]) {
    int		loc = 0;
    int		i;
    memset(image, 0, 80);
    if (*addr == '0') {
	while(*addr != '\0') {
	    loc <<= 3;
	    loc += *addr++ - '0';
	}
    } else {
	while(*addr != '\0') {
	    loc *= 10;
	    loc += *addr++ - '0';
	}
    }
    for(i = 35; i > 20; i--) {
	if (loc & 1)
	   image[i] = 1;
	loc >>= 1;
    }
    wrdnum = 0;
}

/* Add correction data */
int add_data(char *addr, unsigned short image[80]) {
    int long long data = 0;
    int		i;
    int		digit = 2 << (wrdnum / 2);
    int		col = (wrdnum&1)?36:0;

    if (addr == 0) 
	return 0;
    if (*addr == '-') {
	data = (long long)1 << 36;
	addr++;
    }
    if (*addr == '0') {
	while(*addr != '\0') {
	    data <<= 3;
	    data += *addr++ - '0';
	}
    } else if (*addr >= '1' && *addr <= '9') {
	while(*addr != '\0') {
    	    if (*addr < '0' || *addr > '9') 
		return 0;
	    data *= 10;
	    data += *addr++ - '0';
	}
    } else {
	return 0;
    }
    for(i = 35; i >= 0; i--) {
	if (data & 1)
	   image[i+col] |= digit;
	data >>= 1;
    }
    wrdnum++;
    return 1;
}

/* Compute checksum and put word count into correction card */
void checksum(unsigned short image[80]) {
    int long long data = wrdnum;
    int		i;
    int		row;

    /* Word count into correct spot */
    for(i = 17; i > 10; i--) {
	if (data & 1)
	   image[i] |= 1;
	data >>= 1;
    }

    /* Sum up all data on card */
    data = 0;
    for (row = 0; row < 12; row++) {
        long long wd = 0;
	int	 mask = 1 << row;
        for (i = 0; i < 36; i++) {
	    wd <<= 1;
	    if (image[i] & mask)
		wd |= 1;
        }
	data += wd;
	if (data & ((long long)1 << 37))
	    data++;
	data &= ((long long)1 << 37) - 1;
	wd = 0;
        for (i = 0; i < 36; i++) {
	    wd <<= 1;
	    if (image[i+36] & mask)
               wd |= 1;
        }
	data += wd;
	if (data & ((long long)1 << 37))
	    data++;
	data &= ((long long)1 << 37) - 1;
    }

    /* Put sum into correct place */
    for(i = 35; i >= 0; i--) {
	if (data & 1)
	   image[i+36] |= 1;
	data >>= 1;
    }
}

/* Check if card should be copied to output file */
int good_card(unsigned short image[80]) {
    int		   t;
    int		   i, j;
    unsigned long long wd;
    int		   bl;

    ccard++;
    if (image_mode)
	return 1;
    for (t = i = 0; i < 80; i++) t |= image[i];
    if (t == 0) {
        return 0;	/* Ignore blank cards */
    } else  if (ccard < fcard) { /* Before first card? */
	return 0;
    } else if (lcard > 0 && ccard > lcard) { /* After the last card? */
	return 0;
    } else {
	wd = 0;
	t = 1;
	bl = 0;
	/* Check first word of card */
        for (j = 0, i = 0; i < 36; i++) {
     	    wd = (wd << 1) | (image[i] & 1);
     	    t &= image[i] & 1;
	    bl |= image[i] & 1;
        }
	/* Check 9R word */
        for (; i < 72; i++) {
	    bl |= image[i] & 1;
        }
	/* If all ones, skip this */
	if (t) {
	    return 0;
	}
	/* If no data in 9L/9R, skip */
	if (bl == 0 || wd == 0200000000000LL) {
	    return 0;
	}
    }
    return 1;
}

/* Pretty print card for show */
void print_card(unsigned short image[80]) {
    int            t;
    int		  row;
    int            i, j;

    for (t = i = 0; i < 80; i++) t |= image[i];
    if (t == 0) {
        printf("blank\n");
    } else {
        t = 0;
	for(row=0; row < 12; row++) {
	    int mask = 1 << row;
            for (j = 0, i = 0; i < 72; i++) {
		t <<= 1;
		if (image[i] & mask)
                    t |= 1;
                if (j == 2) {
                    putchar('0' + t);
                    t = 0;
                    j = 0;
                } else 
                    j++;
                if (i == 35)
                    putchar(' ');
	    }
            putchar('\n');
        }
        putchar('-');
        putchar('\n');
    }
}



int
main(int argc, char *argv[]) {
    unsigned short image[80];
    int		   col;
    int		   t;
    int		   i, j;
    unsigned char  ch;
    int		   card;
    char	   *n;
    FILE	   *of = stdout;
    FILE	   *f;

    card = 0;
    while((n = *++argv) != NULL) {
	if (*n == '-') {
	    switch (n[1]) {
	    case 'o':   /* Output file */
		n = *++argv;
		if ((of = fopen(n, "wb")) == NULL) {
		    fprintf(stderr, "Unable to open: %s for output\n", n);
		    exit(1);
		}
		break;
	    case 't':   /* Tranfer card */
		transfer_card(*++argv, of);
		break;
	    case 'B':	/* Truely Blank card. */
                memset(image, 0, sizeof(image));
		{
		   int seq = sequence;
		   char *lab = label;
		   sequence = 0;
		   label = NULL;
		   dump_card(of, image);
		   sequence = seq;
		   label = lab;
		}
		break;
	    case 'b':	/* Blank card */
                memset(image, 0, sizeof(image));
		dump_card(of, image);
		break;
	    case 'a':   /* Ascii text card */
		n = *++argv;
		if ((f = fopen(n, "r")) == NULL) {
                    fprintf(stderr, "Unable to open: %s\n", n);
                    exit(1);
                }
		dump_text(of, f);
		fclose(f);
		break;
	    case 'i':	/* Toggle image mode */
		image_mode = !image_mode;
		break;
	    case 'l':	/* Set label */
		label = *++argv;
		break;
	    case 's':	/* Sequence */
		sequence = ! sequence;
		break;
	    case 'c':   /* Correction card */
		memset(image, 0, sizeof(image));
		set_addr(*++argv, image);
		while(add_data(*++argv, image)); 
		argv--;	/* Back up one argumement */
		checksum(image); /* Compute checksum */
		dump_card(of, image);
		break;
	    case 'f':	/* Start card */
		++argv;
		if (*argv[0] == 't') 
		    fcard = -1;
		else
		    fcard = atoi(*argv);
		break;
	    case 'e':	/* Final card */
		++argv;
		if (*argv[0] == 't') 
		    lcard = -1;
		else
		    lcard = atoi(*argv);
		break;
	    }
	} else {
	    if ((f = fopen(n, "rb")) == NULL) {
               fprintf(stderr, "Unable to open: %s\n", n);
               exit(1);
            }

            memset(image, 0, sizeof(image));
            col = 0;
            t = 6;
            while(!feof(f)) {
                ch = fgetc(f);
	        if (ch & 0x80) {
	           if (col != 0) {
	                if (good_card(image)) {
			    dump_card(of, image);
	                    card++;
			}
                    }
	            memset(image, 0, sizeof(image));
	            col = 0;
	            t = 6;
	        }
	        image[col] |= (ch & 077) << t;
	        if (t) {
	            t = 0;
	        } else {
	            col++;
	            t = 6;
		}
           } 
           fclose(f);
           if (col != 0) {
	      if (good_card(image)) {
	          dump_card(of, image);
	          card++;
	      }
           }
	   ccard = 0;
	   fcard = 0;
	   lcard = -1;
         }
     }
   fprintf(stderr, "%d cards in deck\n", cardno);
   return (0);
   
}

