/* sdsnm.c - produce namelist for SDS binary object file   */

/* created by Ken Rector, Aug 23, 2020 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define eof 027657537


FILE *infile, *outfile;

const char sds930_to_ascii[64] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', ' ', '=', '\'', ':', '>', '%',            /* 17 = check mark */
    '+', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
    'H', 'I', '?', '.', ')', '[', '<', '@',             /* 37 = stop code */
    '-', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', '!', '$', '*', ']', ';', '^',             /* 57 = triangle */
    ' ', '/', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', '?', ',', '(', '~', '\\', '#'             /* 72 = rec mark */
};                                                 /* 75 = squiggle, 77 = del */

typedef struct df {
    int lbl1;
    int lbl2;
    int value;
    int bias;
} df;

df  def[1000];
df  adef[1000];
df  ref[1000];
int ddd = 0;
int addd = 0;
int rddd = 0;
int bias = 0;
int fbias = 0;

unsigned int get24(){
    unsigned int i;
    
    i = fgetc(infile) & 077;
    i = i<<6 | (fgetc(infile) & 077);
    i = i<<6 | (fgetc(infile) & 077);
    i = i<<6 | (fgetc(infile) & 077);
    return(i);
}

unsigned int get24bcd(){
    unsigned int i;
    
    i = sds930_to_ascii[fgetc(infile) & 077];
    i = (sds930_to_ascii[fgetc(infile) & 077]) << 8 | i;
    i = (sds930_to_ascii[fgetc(infile) & 077]) << 16 | i;
    i = (sds930_to_ascii[fgetc(infile) & 077]) << 24 | i;
    return(i);
}

int cmpfnc (const void * a, const void * b) {
    df  *x = (df *)a;
    df  *y = (df *)b;
    return ( x->value - y->value );
}


int main(int argc, char *argv[]) {
    unsigned int i,j,n;
    unsigned int bin,type,cksm,cnt;
    int len;
    int max;
    int nf;
    
    int l1;
    int l2;
    int val;
    char length[24];
    char *lll;
    
    if (argc < 3)
    {
        printf("Usage: sds_nm bias binfile ... \n");
        printf("where bias is an octal value defining the origin of\n");
        printf("the first file in a list of object files\n");
        printf("binfile ... is a list of SDS standard binary object files\n");
        exit(1);
    }
    
    i = sscanf(argv[1], "%o", &bias);
    if ( i != 1)
        printf("invalid bias\n");
    outfile = stdout;
    
    printf("\n\n           EXTERNAL DEFS \n");
    for (nf = 2;nf < argc;nf++) {
        infile = fopen(argv[nf], "rb");
        if (!infile)         {
            printf("Cannot open %s\n", argv[nf]);
            return 1;
        }
        
        do {
            len = 40;
            i = get24();                    // control word
            len--;
            if (feof(infile))
                break;
            if (i!=eof) {
                if ((bin = (i >> 12) & 07) != 05) {
                    for (j = 0; j < len;j++) { // skip non binary card
                        i = get24();
                    }
                    continue;
                }
                type = i >> 21;             // control type
                cnt = (i >> 15) & 077;
                cnt--;                      // count this control word
                cksm = i & 07777;
                switch (type) {
                    case 0:                 // data
                        while (cnt) {
                            i = get24();
                            cnt--;
                            len--;
                        }
                        for (j = 0;j < len;j++)
                            i = get24();
                        break;
                    case 3:                 // end
                        max = get24() & 077777;   // save max location
                        cnt--;
                        len--;
                        while (cnt) {
                            i = get24();
                            cnt--;
                            len--;
                        }
                        for (j = 0;j < len;j++)
                            i = get24();
                        break;
                    case 1:                 //ext def/ref
                        while(cnt) {
                            l1 = get24bcd();
                            cnt--;
                            len--;
                            l2 = get24bcd();
                            cnt--;
                            len--;
                            val = get24();
                            cnt--;
                            len--;
                            def[ddd].value = n & 077777;
                            switch (val >> 22) {
                                case 0:         // common or program length
                                    break;
                                case 1:         // external ref
                                    for (j = 0; j < rddd;j++) {
                                        if ((l1 == ref[j].lbl1) &&
                                            (l2 == ref[j].lbl2))
                                            break;
                                    }
                                    if (j == rddd) {
                                        ref[rddd].lbl1 = l1;   // add ref to list
                                        ref[rddd].lbl2 = l2;
                                        ref[rddd].value = nf;
                                        ref[rddd].bias = bias;
                                        rddd++;
                                    }
                                    break;
                                case 2:         // external def
                                    if (val & 0100000) {
                                        def[ddd].lbl1 = l1;
                                        def[ddd].lbl2 = l2;
                                        def[ddd].value = (val & 077777777);
                                        def[ddd].bias = bias;
                                        ddd++;
                                    }
                                    else {
                                        adef[addd].lbl1 = l1;
                                        adef[addd].lbl2 = l2;
                                        adef[addd].value = (val & 077777777);
                                        addd++;
                                    }
                                    break;
                                case 3:         // external ref w/added item
                                    break;
                            }
                            
                        }
                        for (j = 0;j < len;j++)
                            i = get24();
                        break;
                    default:
                        printf("unknown record type  %o\n",i);
                }
            }
            n++;
        } while(!feof(infile));
        
        qsort(def,ddd,sizeof(df),cmpfnc);
        qsort(adef,addd,sizeof(df),cmpfnc);
        

        printf("\nFile:  %s  Load Address: %05o\n",argv[nf],bias);
        if (ddd) {
            printf("Relocatable:\n");
            for (j = 0; j < ddd; j++) {
                printf("\t");
                char *c = (char *)&def[j];
                for (i = 0; i < 8; i++)
                    printf("%c",*c++);
                printf("    %05o",def[j].value & 077777);
                printf("    %05o\n",(def[j].value+def[j].bias) & 077777);
            }
        }
        if (addd) {
            printf("\n Absolute:\n");
            for (j = 0; j < addd; j++) {
                printf("\t");
                char *c = (char *)&adef[j];
                for (i = 0; i < 8; i++)
                    printf("%c",*c++);
                printf("    %05o\n",adef[j].value & 077777);
            }
            printf("\n");
        }
        if (rddd) {
            printf("\n Unsatisfied Refs:\n");
            for (j = 0; j < rddd; j++) {
                for (n = 0; n < ddd; n++) {
                    if ((ref[j].lbl1 == def[n].lbl1) &&
                        (ref[j].lbl2 == def[n].lbl2))
                        break;
                }
                if (n == ddd) {
                    for (n = 0; n < addd; n++) {
                        if ((ref[j].lbl1 == adef[n].lbl1) &&
                            (ref[j].lbl2 == adef[n].lbl2))
                            break;
                    }
                    if (n == addd) {
                        printf("\t%s   %s\n",(char *)&ref[j],argv[ref[j].value]);
                    }
                }
            }
        }
        fclose(infile);
        ddd = 0;
        addd = 0;
        rddd = 0;
        bias += max;
    }
    fclose(outfile);
}

