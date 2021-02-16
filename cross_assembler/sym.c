//
//  sym.c
//  
//
//  Created by Ken Rector on 3/15/20.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "sym.h"

extern tbl *scan(void);
extern void scnr(void);
extern void scnm(void);
extern int  scnl(void);
extern tbl mt[];
extern tbl sym[];
extern tbl lt[];
extern tbl rt[];
extern uint8_t ascii_to_sds930[];
extern int ctt[];

FILE *fp;
FILE *ifp;
FILE *lofp;
FILE *bofp;

char *lbl;              // loc string
char *op;               // op string
char *var;              // operand string
char crd[84];
char line[84];
char refr[8];
int  lineno;
tbl  lbl1;


int  sot[16];
int  sit[16];
int  it[16];
int  sxt[4];
int  scxt[4];


int  *lop;               // ptr to loc of scan connector
int  *litm;
int  *lsot = sot;      // location of scan connnector
int  *lsit = sit;       // location of scan item
int  *lsxt = sxt;
int  *scnx = sxt;

int icn;
int cw;
int derr;
int dw[40];
int dwc;                // record size
int eerr;
int ierr;
int iflg;
int inst;
int ladd;               // current literal pool location
int lerr;
int loc;
int locp;               // literal pool origin
int mloc;
int mode;
int oerr;
int octf;
int pass2;
int perr;
int ploc;
int pmod;
int prel;
int ptyp;
int qloc;
int rel;
int relm;
int rerr;
int scno;
int sm1;
int srel;
int styp;
int terr;
int term;
int uerr;
int _verr;
int valu;
int wmod;
int word;
int wrd2;               // form data
int wrd3;
int xerr;
int xflg;

int ct[] = {0,0100,020000000,020000100};

void dir(int op);

void edtd(int i);

unsigned int parity7[64] = {
    0100, 0000, 0000, 0100, 0000, 0100, 0100, 0000    /* 00 - 07 */
    ,0000, 0100, 0100, 0000, 0100, 0000, 0000, 0100    /* 10 - 17 */
    ,0000, 0100, 0100, 0000, 0100, 0000, 0000, 0100    /* 20 - 27 */
    ,0100, 0000, 0000, 0100, 0000, 0100, 0100, 0000    /* 30 - 37 */
    ,0000, 0100, 0100, 0000, 0100, 0000, 0000, 0100    /* 40 - 47 */
    ,0100, 0000, 0000, 0100, 0000, 0100, 0100, 0000    /* 50 - 57 */
    ,0100, 0000, 0000, 0100, 0000, 0100, 0100, 0000    /* 60 - 67 */
    ,0000, 0100, 0100, 0000, 0100, 0000, 0000, 0100};  /* 70 - 77 */





int *errflg[] = {&xerr,&oerr,&derr,&eerr,&ierr,&lerr,&perr,
    &rerr,&terr,&uerr,&_verr};
char errchr[] = {'*','O','D','E','I','L','P','R','T','U','V'};

// output word to card punch
void punch(int fst, int data)  {
    int tmp;
    
    for (int i = 18; i >= 0;i-=6) {
        tmp = ((data >> i ) & 0x3f);
        tmp = tmp | parity7[tmp];
        if ((fst == 0) & (i == 18))
            tmp |= 0x80;        // Set record mark
        fwrite(&tmp,1,1,bofp);
    }
}

// save item into table - clear next entry
void _move(tbl *item,char *key,int l,int m) {
    if (*(int *)item != (int)item)
        printf("move error %s\n",item->lbl);
    memcpy(item->lbl,key,6);
    item->word = l;                 // save loc address and mode
    item->mode = m;
    item++;
    *(int *)item = (int)item;       // mark end of table
    return;
}

tbl *srch(tbl *t, char *key) {
    int lit;
    if (t == lt)
        lit = 1;
    else
        lit = 0;
    while (*(int *)t != (int)t) {
        if (!lit) {
            if (strncmp(t->lbl,key,6) == 0)
                return t;
        }
        else {
            if (*(int *)t == *(int *)key)
                return t;
        }
        t++;
    }
    return t;                   // return address of ending entry
}


// insert item onto table
tbl *nsrt(tbl *t,char *key,int l,int m) {
    tbl *item;
    
    item = srch(t,key);
    if (*(int *)item == (int)item) {     // not found
        if (!pass2)
            _move(item,key,l,m);
    }
    else {
        if (!pass2)                     // found
            if (item->word != l)
                derr++;
    }
    return item;
}

//define label at given location
void dlbl(char *key, int l) {
    tbl *item;
    
    if (lbl1.lbl[0] != '\0') {
        item = nsrt(sym,key,l,lbl1.mode);
    }
}

void reset(int typ) {
    qloc = loc;                       // save  record load addresss
    ptyp = typ;                       // save record type
    rel = srel = prel = 0;
    
    return;
}

// compute folded checksum
int chksum(int *ctl) {
    int ck = 0;
    int chks;
    int n;
    
    n = ((cw >> 15) & 077);         // nr words in record
    ck = (cw & 077770000) ^ 077770000;
    for (int i = 0; i < (n-1);i++)
        ck ^= (ctl[i] &077777777);      // 24 bit words
    chks = (ck >> 12) & 07777;
    chks ^= ck & 07777;
    return chks;
}

// build cpntrol word and other controls for output record
void flush () {
    int i;
    
    if ((ptyp == RECDAT) | (ptyp == RECEND)) {  // record type being flushed
        dwc++;                        // include load-address word in count
        dw[0] = qloc;                 // store load-address
        if (rel) {
            dw[dwc++] = rel;          //store relocation indicator word
            dw[0] |= 02000000;        // include load-relocation word in count
        }
        if (srel) {
            dw[dwc++] = srel;         // include special reloc word in count
            dw[0] |= 020000000;
        }
        if (prel) {
            dw[dwc++] = prel;
            dw[0] |= 010000000;
        }
    }
    cw = (((ptyp << 6) + dwc + 1) << 15) | 050000;  // initialize record control word
    cw += chksum(dw);                   // compute folded checksum
    punch(0,cw);
    for (i = 0; i < dwc; i++) {
        punch(1,dw[i]);
    }
    for (;i < 39;i++)                   // 40 - ce card
        punch(1,0);
    
    dwc = 0;
    return;
}

// output word
void outp(int ctyp, int styp, tbl *t, int loc) {
    char  lbl[8];
    int   i;
    
    if ((dwc & 077777) == 0) {          // empty record, reset only
        reset(ctyp);
    }
    else if (ctyp != ptyp) {            // type change
        flush();                        // flush out current record
        reset(ctyp);
    }
    else if (dwc >= 21) {               // record filled
        flush();                        // flush out current record
        reset(ctyp);
    }
    else if (ptyp == RECDAT) {          // previous type was data record
        if (loc != (ploc + 1)) {        // if not incremental location
            flush();                    // flush out current record
            reset(ctyp);
        }
    }
    switch (ctyp) {                     // switch on current record type
        case RECEND+1:                   // endn
            if ((cw >> 21) != RECEND) { // do nothing when previous type was end
                ptyp = RECEND;
                qloc = mloc;
                flush();
            }
            break;
        case RECEND:                     // endm - end record with transfer address
            word = (*(int *)t & 077777) | 00100000;  // bru op + trannsfer address
            qloc = mloc;                // max value of location counter + 1
            //printf("type 3:  %08o  %08o\n",word,qloc);
            t = (tbl *)&word;           // fall thru to finish as data
        case RECDAT:                         // data record
            ploc = loc;
            dw[dwc+1] = *(int *)t;  // leave the load/transfer word empty for now
            rel |= (wmod & 02)  << (22 - dwc);
            if (styp)
                srel += 1 << (24-dwc);      // special i/o word
            if (pmod != 0)
                prel += 010000000 >> dwc;
            //printf("type 0:  %08o  %08o\n",dw[dwc],loc);
            dwc++;
            break;
        case RECDEF:                         // def/ref record with subtype
            for (i = 0; i < 6; i++) {
                if (t->lbl[i] == '\0')
                    break;
                lbl[i] = ascii_to_sds930[t->lbl[i]];
            }
            for (;i < 8; i++)
                lbl[i] = ascii_to_sds930[' '];
            dw[dwc] = dw[dwc+1] = 0;
            for (int i = 0; i < 4;i++) {
                dw[dwc] = dw[dwc] << 6 | lbl[i];
                dw[dwc+1] = dw[dwc+1] << 6 | lbl[i+4];
            }
            dw[dwc+2] = t->word | (styp << 22);
            //printf("type 1: %08o   %08o    %08o\n",
            //       dw[dwc],dw[dwc+1],dw[dwc+2]);
            dwc += 3;
            break;
        case RECPOP:                         // pop ref/def record with subtype
            //word = cntr;
            //dw[dwc] = word;
            //dw1[1] = (*word++ & 077770000) | '  ';
            //dw1[dwc+2] = *word;
            break;
    }
    return;
}



void maxl() {
    if (mloc  < loc)
        mloc = loc;
}

// increment location value
int iloc(int l) {
    maxl();                // set maximum loc
    scan();                 // get increment
    return  l + valu;
}


// count bits in field
// wrd2 = form word
// wrd3 = B register for NOD
// returns bit count, >24 if last field
int fldc() {
    int i;
    int A,B;
    int X;
    
    A = wrd2 ^ 040000000;
    B = wrd3;
    X = 22;
    for (i = 0; i < 24; i++,X--) {  // simulator  NOD 24
        if ((A ^ (A << 1)) & 040000000)
            break;
        A = ((A << 1) | (B >> 23)) & 077777777;
        B = (B << 1) & 077777777;
    }
    wrd2 = A << 1;          // beginning of next field to A[0]
    wrd3 = B;
    return (23 - X) & 077777;
    // when A == 0, nod returns X == 22 - 24 == -2
    // this returns 23 - (-2) = 25
}
/*
 
 for (i = 0; i < sc; i++) {                  // until max count
 if ((A ^ (A << 1)) & SIGN)
 break;
 A = ((A << 1) | (B >> 23)) & DMASK;
 B = (B << 1) & DMASK;
 }
 X = (X - i) & DMASK;
 */



void fref() {               // 811
    int cnt1;
    int cnt2;
    int cnt3;
    
    cnt1 = 24;              // no. of bits left
    cnt3 = -5;              // set field count
    wrd2 = word;            // form word for fdlc()
    wrd3 = 040000000;
    word = 0;               // 0 to data
    do {
        if ((cnt2 = fldc()) > 24) // fr4:  count bits in field
            break;           // no more fields
        cnt3++;              // count fields
        // if it is address field size or last field
        if ((cnt2 == 15) && (cnt2 == cnt1)) {
            valu = scnl();   // scan as address field
        }
        else {
            scan();   // get non-address field - valu in a and mode in b
            if (relm)
                rerr++;      // set R flag
        }
        //valu = valu >> cnt2;   // left adjust valu in B
        if ((valu >> cnt2) != 0) {
            if (valu >> cnt2 != -1) {
                terr++;         //fr6
            }
        }
        //valu = valu << (24 - cnt1);
        word = (word << cnt2) | valu;     // insert field in word
        cnt1 = cnt1 - cnt2;     // decrement bits left
    } while (term == ',');      // process next field
    if (cnt3 < 0)
        wrd2 = 0;               // too many fields to edit
    if (pass2) {
        edtd(0);       // edit instruction form
        outp(RECDAT,0,(tbl *)&word,loc);
    }
    return;
}

void ede() {
    int i = 0;
    for (int j = 0;j < 11;j++) {
        if (*errflg[j]) {
            fprintf(lofp,"%c",errchr[j]);
            i++;
            *errflg[j] = 0;
        }
    }
    for (;i<5;i++)
        fprintf(lofp," ");
    return;
}
void edit(int form,int word) {
    ede();
    fprintf(lofp,"%05o",loc & 037777);
    fprintf(lofp,"  %01o %02o %01o %05o",(word>>21)&7,(word>>15)&077,(word>>14)&01,word&037777);
    fprintf(lofp,"  %6d",lineno);
    fprintf(lofp,"  %s",crd);
    fprintf(lofp,"\n");
}

void edti() {
    ede();
    fprintf(lofp,"%05o",loc & 037777);
    fprintf(lofp,"  %01o %02o %05o  ",(word>>21)&7,(word>>15)&077,word&077777);
    fprintf(lofp,"  %6d",lineno);
    fprintf(lofp,"  %s",crd);
    fprintf(lofp,"\n");
}

void edtl() {
    ede();
    fprintf(lofp,"%05o",loc & 037777);
    fprintf(lofp,"              ");
    fprintf(lofp,"  %6d",lineno);
    fprintf(lofp,"  %s",crd);
    fprintf(lofp,"\n");
}

void edtv(int op) {
    ede();
    if (op)
        fprintf(lofp,"       %08o    ",word);
    else
        fprintf(lofp,"                   ");
    fprintf(lofp,"  %6d",lineno);
    fprintf(lofp,"  %s",crd);
    fprintf(lofp,"\n");
}

void edtd(int i) {
    ede();                          // 5 bytes of error code
    fprintf(lofp,"%05o",loc & 037777);
    fprintf(lofp,"  %08o    ",word & 077777777);
    if (!i) {
        fprintf(lofp,"  %6d",lineno);
        fprintf(lofp,"  %s",crd);
    }
    fprintf(lofp,"\n");
}

void edtr(char *s,int l) {
    fprintf(lofp,"     %05o",l & 037777);
    fprintf(lofp,"  %s",s);
    fprintf(lofp,"\n");
}


// handle INS2 and INS9
void inr3() {
    int b,a;
    
    if (mode & 04000) {      //   INS9 - 00004010
        scan();             // :360  first arg = channel, valu in a and mode in b
        if (valu & 04)
            b = 00600000;
        else
            b= 0;
        a = b | ct[valu * 03] | word;
        if ((a & 07700000) == 04600000)
            a = a ^ 00640000;
        word = a;
        if (term != ',') {
            wmod = 0;
            edit(044040000,word);       // edit instruction form
            outp(RECDAT,0,(tbl *)&word,loc);           // type 0 output item
            return;             // only one arg
        }
    }
    if (iflg)                    // :377  inr31  INS2, or two args
        word |= 040000;         // set indirect bit
    scan();                     // next arg - valu in a and mode in b
    if (valu & 077777700)
        terr++;
    word += (valu & 077);           // :384 insert unit
    if (term == ',') {
        scan();                 // second or third arg - valu in a and mode in b
        if (valu != 0)
            valu -= 1;
        word |= (valu & 03) << 7;    // position char/word
    }
    wmod = 0;
    edti();   //edit(044040000,word);       // edit instruction form
    outp(RECDAT,0,(tbl *)&word,loc);           // type 0 output item
    return;
}

// mark this label as external if it exists
void lxl(char *l) {
    tbl *t;
    t = srch(sym,l);
    if (*(int *)t != (int)t)
        t->mode |= xflg;
    return;
}

int lbl1p1,lbl1p2;


// build pop insruction
void popi() {
    
#ifdef NEVER
    tbl t;
    
    dlbl(lbl1.lbl,loc);        // define label
    word = 0;
    if (!pass2) {
        loc++;
        lbl1p2 = 020000000;
        memcpy(&sym[0],&lbl1p1,3);
        lbl1p1 += 02040;        // popd1
    }
    else {
        dir(PZE);
    }
#endif
    printf(" POPI called at line %d\n", lineno);
    return;
}

void popd() {
    if (!pass2) {
        lbl1p2 = loc;
        lbl1p1 = lbl1p1 & 077777776;
        if (xflg)
            lbl1p2 += 040000000;
        lbl1p1 += 02040;        // like popd1
    }
    else {
        //BRM     PRNT
    }
}

void popr() {
    pmod++;
    word = ((word & 017600000) | 020000000) >> 1;
    ierr++;
    dir(PZE);
    return;
}




// end card
void end() {
    tbl *t;
    int eflg;
    int ltr1;
    
    if (!pass2) {
        rewind(ifp);        // end1
        pass2 = -1;
        locp = loc;          // set literal pool origin
        ladd = loc;          // set 1st literal address
        xerr = oerr = derr = eerr = ierr = lerr = perr = 0;
        terr = uerr = _verr = 0;
        dwc = 0;
        t = sym;                            // end14  search symbol table
        while (*(int *)t != (int)t) {
            if (t->mode & EXTM) {
                t->mode &= ~EXTM;               // reset external flag
                outp(RECDEF,DEFT,t,loc);       // type 1 output def
            }
            t++;
        }
        ltr1 = 0;                   // end13
        t = lt;                     // search literal table
        while (*(int *)t != (int)t) {
            if ((t->mode & 07777) == 02040) {   // if TYP or TYPW
                t->word |= (ltr1 << 16);
                ltr1++;
                outp(RECPOP,0,t,loc);
            }
            t++;
        }
        loc = 0;
    }
    else {
        eflg = 0;                   // end2  -  pass 2
        if (strlen(var)){
            scan();                 // get transfer address - valu in a and mode in b
            word = valu;
            //trns = valu;
            edtv(END);
            eflg++;
        }
        else
            edtv(0);
        loc = locp;                 // end5
        ltr1 = locp;                // literal pool origin
        t = lt;
        while (ltr1 != ladd) {      // end6  check if f 0s in pool
            word = *(int *)&(t->lbl);   // value
            mode = t->mode;
            wmod = mode;
            edtd(1);                    // no line num or source
            outp(RECDAT,0,(tbl *)&word,loc);
            ltr1++;                     // increment sequence
            loc++;                      // include lit in data
            t++;
        }
        maxl();                         // set max load address after literals
        t = rt;
        while (*(int *)t != (int)t) {   // end12
            edtr(t->lbl,t->word);
            outp(RECDEF,REFT,t,loc);   // type 1 output ref
            t++;
        }
        if (eflg--) {                     // end15
            //word = valu;
            wmod = mode;
            outp(RECEND,0,(tbl *)&valu,loc);     // type 3 - output transfer card
        }
        outp(RECEND+1,0,(tbl *)&valu,loc);      // end7  type 4 no transfer - output clear cards
        return;                            // end11
    }
}

// process directives
void dir(int op) {
    tbl  *t;
    char *tmp;
    int i = 0;
    int j;
    int m;
    int n;
    
    
    switch (op) {
        case 0:                         // machine instruction
            printf(" machine instruction 0\n");
            break;
        case INS2:
        case DIR1:
        case DIR2:
            printf(" mnemonic opcode error  %o\n",op);
            break;
        case BORG:
            octf = 0;
        case AORG:
        case ORG:                       // same as RORG
            if (op == AORG)
                loc = iloc(0);
            else
                loc = iloc(00100000);   // get new loc  relocatable
            dlbl(lbl1.lbl,loc);        // define label
            if (pass2)
                edtl();
            break;
        case BSS:                       //  RES
            n = iloc(0);
            dlbl(lbl1.lbl,loc);        // define label
            if (pass2)
                edtl();
            loc += n;
            break;
        case COPY:
        case DATA:
            i = 0;
            do {
                scan();             // valu in a and mode in b
                word = valu;
                if (pass2) {
                    wmod = mode;
                    edtd(i);
                    outp(RECDAT,0,(tbl *)&word,loc);           // type 0 output item
                }
                i++;
                loc++;
            }  while (term == ',');
            break;
        case DEC:
            m = *var;
            if (sscanf(var,"%d",&word) != 1)
                eerr++;
            word &= 077777777;
            if ((word == 0) && (m == '-'))
                word = 040000000;
            if (pass2) {
                edtd(0);
                //outp(RECDAT,0,(tbl *)&word,loc);           // type 0 output item
            }
            loc++;
            break;
        case DED:
        case END:
            end();
            // bgn4 - begine pass2
            mloc = 0;
            loc = 00100000;                 // relocatable 00000
            break;
        case BOOL:
            octf = 0;
        case EQU:                       // 435
            scan();                     // get value
            word = valu;
            t = nsrt(sym,lbl1.lbl,
                     (mode & RELM) ? valu |00100000 : valu,
                     (mode & 077777776) | xflg); // insert in symbol table
            if (pass2)
                edtv(EQU);
            break;
        case FORM:                  // 466
            
            wrd2 = 0;
            do {
                scan();    // get field length - valu in a and mode in b
                wrd2 = (wrd2 | 01)<< (valu);
            }  while (term == ',');     // process next field
            lbl1.mode += 041;
            word = wrd2>>1;
            mode = 041;
            t = nsrt(mt,lbl1.lbl,word,mode);  // insert form item
            if (pass2) {
                edtd(0);       // edit instruction form
                outp(RECDAT,0,(tbl *)&word,loc);
            }
            
            break;
        case NOPO:
        case OCT:
            j = 0;
            do {
                n = 0;
                m = *var;
                if ((m == '+') || (m == '-'))
                    var++;
                for (i = 0; i < strlen(var);i++) {
                    term = var[i];
                    if ((var[i] == ' ') || (var[i] == ',')){
                        var[i] = '\0';
                        break;
                    }
                    if (!isdigit(var[i])) {
                        break;
                    }
                    if ((var[i] == '8') || (var[i] == '9'))
                        n++;
                }
                if ((i == strlen(var)) && !n)
                    n = sscanf(var,"%o",&word);
                else {
                    n = 0;
                    word = 0;
                }
                if (term == ',')
                    var = var + strlen(var) + 1;
                word &= 077777777;
                if ((word == 0) && (m == '-'))
                    word = 040000000;
                if (!n)
                    eerr++;
                if (pass2) {
                    edtd(j++);
                    outp(RECDAT,0,(tbl *)&word,loc);           // type 0 output item
                }
                
                loc++;
            }  while (term == ',');
            break;
        case OPD:
            octf = FORCEO;          // set octal flag
            scan();                 // get value  - valu in a and mode in b
            word = valu;
            lbl1.mode += 040;       // programmer defined mnemonic
            t = nsrt(mt,lbl1.lbl,valu,0);
            if ((valu & 07777) == 02140) {
                t->mode = lbl1.mode;
                t->word = lbl1.word;
            }
            dlbl(lbl1.lbl,loc);                 // define label
            if (pass2) {
                edtv(OPD);
            }
            break;
        case PAGE:
            break;
        case POPD:
        case PZE:
            if (pass2) {
                word = (scnl() & 077777);             // inr1 - get instruction address
                wmod = mode;
                // EOM or EOD
                if (iflg)
                    word |= 040000;         // inr92 - set indirect bit
                // inr94
                octf = 0;                  // reset octal flag
                if (term == ',') {
                    scan();                 // get index - valu in a and mode in b
                    word |= (valu & XMASK) << 21; // inr2  insert index
                }
                if ((term != ' ')  && (term != '\0'))       // lin8  test if blank or \0 terminator
                    eerr++;
                term = ' ';
                edit(044040000,word);       // edit instruction form
                outp(RECDAT,0,(tbl *)&word,loc);           // type 0 output item
            }
            loc++;
            break;
        case BPT:
            if (pass2) {
                word = 04020000;            // base sks instruction
                do {
                    scan();                    // get break-point
                    word = word + (01000 >> valu);
                }  while (term == ',');
                edit(0,word);       // edit instruction form
                outp(RECDAT,0,(tbl *)&word,loc);
            }
            loc++;
            break;
        case BCI:
            // goto text1;
        case BCD:
        case TEXT:
            if (*var == '<') {
                tmp = strtok(++var,">");
                n = (int)strlen(tmp);
            }
            else {
                tmp = strtok(var,",");   // text1   get count
                if (tmp == NULL) {
                    terr++;
                    n = 0;
                }
                else if (*tmp == '0')
                    i = sscanf(tmp,"%o",&n);
                else
                    i = sscanf(tmp,"%d",&n);
                if (i != 1)
                    terr++;
                tmp = strtok(NULL,"");
                m = strlen(tmp);
            }
            i = 0;
            mode = 0;
            wmod = 0;
            for (int j = 0;j < n;) {
                word = 0;
                for (int k = 0;k<4;k++,j++) {
                    if (j < m)
                        word = (word<<6) | ascii_to_sds930[tmp[j]];
                    else
                        word = (word<<6) | ascii_to_sds930[' '];         //60;
                }
                if (pass2) {
                    edtd(i++);
                    outp(RECDAT,0,(tbl *)&word,loc);           // type 0 output item
                }
                loc++;
            }
            break;
        default:
            printf("unknown pseudo op %d\n",op);
            break;
    };
}



// read line
int _read()  {
    int tmp;
    
    //ssize_t r;
    //size_t len = 0;
    int i;
    int c;
    char *p;
    FILE *infile;
    
    if (pass2) {
        infile = ifp;
    }
    else {
        infile = fp;
    }
    while (1) {
        tmp = 0;
        p = line;
        while ((c = fgetc(infile)) != EOF) {
            if (c == '\r')
                continue;
            if (c == '\n') {
                *p++ = '\0';
                break;
            }
            if (c == '\t') {
                for (int j = 0; j < 4;j++)
                    *p++ = ' ';
            }
            else
                *p++ = toupper(c);
            if (c != ' ')
                tmp++;
        }
        if (c == EOF)
            return -2;
        if (tmp)
            break;
    }
    
    if(!pass2)
        fprintf(ifp,"%s\n",line);
    if (c == EOF)
        return -2;                 // end of file or error
    
    strcpy(crd,line);
    if (line[0] == '*') {
        return -1;                     // REMARK card
    }
    if (line[0] != ' ') {
        if (line[0] == '$') {
            if (!pass2)
                xflg = EXTM;                 // set external flag
            lbl = strtok(&line[1]," ");
        }
        else
            lbl = strtok(&line[0]," ");
        op = strtok(NULL," ");
        if (lbl[0] == '*') {
            if (pass2)
                edtv(0);               // REMARK card
            return -1;
        }
    }
    else {
        lbl = "";
        op = strtok(line," ");
    }
    //    if (op == NULL)
    //        return 0;
    var = strtok(NULL,"");
    if (var == NULL)
        var = "";
    i = 0;
    while (*var == ' ') {
        i++;
        var++;        // skip leading blanks
    }
    if (i > 7)
        var = "";
    return (int)strlen(crd);
}

int main(int argc, char **argv) {
    
    int i;
    int r;
    tbl *mne;
    char *c;
    
    if (argc < 2 || argc > 4) {
        printf("Usage: sym source-file [[list-file][-]] [[binary-file]]]");
        exit(1);
    }
    fp = fopen(argv[1],"r");
    if (fp == NULL) {
        fprintf(stderr,"Open error on source-file: %s\n",argv[1]);
        exit(1);
    }
    if (argc > 2) {
        if (argv[2][0] != '-') {
            lofp = fopen(argv[2],"w");
            if (lofp == NULL) {
                fprintf(stderr,"Open error on list-file: %s\n",argv[2]);
                exit(1);
            }
        }
        else
            lofp = fopen("/dev/null","w");
    }
    if (argc > 3) {
        bofp = fopen(argv[3],"w");
        if (bofp == NULL) {
            fprintf(stderr,"Open error on binary-file: %s\n",argv[3]);
            exit(1);
        }
    }
    else
        bofp = fopen("/dev/null","w");
    
    ifp = fopen("sym_i","w+");
    
    pass2 = 0;
    loc = 00100000;                     // relocatable 0000
    mloc = 0;
    lineno = 0;
    lop = lsot;
    litm = lsit;
    scnx = lsxt;
    *(int *)sym = (int)sym;
    *(int *)lt = (int)lt;
    *(int *)rt = (int)rt;
    mne = srch(mt,"QQQ");
    *(int *)mne = (int)mne;
    
    while (1) {
        iflg = 0;
        xflg = 0;
        octf = 0;                      // reset octal flag
        lbl1.lbl[0] = '\0';
        if (pass2)
            lineno++;
        
        if ((r = _read()) == -2)          // read line
            break;                          // EOF
        if (r == -1) {
            if (pass2)
                edtv(0);
            continue;                       // comment card
        }
        if (r == 0)
            continue;
        
        if (strlen(lbl)) {            // lin1  process label field
            lbl = strtok(lbl,",");
            while (1) {
                //if (*lbl == '0')
                //   lerr++;
                
                if (!(CTT(*lbl) & SC))
                    lerr++;
                strcpy(lbl1.lbl,lbl);
                lbl1.mode = xflg;
                if (strpbrk(lbl,"'+-()*/$") != NULL) // illegal character
                    lerr++;
                for (i = 0; i< strlen(lbl);i++)
                    if (!isdigit(lbl[i]))         // test for at least
                        break;                   // 1 alphabetic
                if (i == strlen(lbl))
                    lerr++;                     // all numeric
                if (strlen(lbl) > 6) {
                    lbl[6] = '\0';                // too long
                    lerr++;
                }
                lxl(lbl);                // if label exists, or in xflg
                lbl = strtok(NULL,",");
                //if ((c = strpbrk(lbl,",")) == NULL) {
                if (lbl == NULL) {
                    //if (lerr)                   // test for label error
                    //lbl1.lbl[0] = '\0';     // don't enter label
                    break;
                }
                
                //lbl = c + 1;            // processs next label
            }
        }
        //lxl(lbl);                           // if label exists, or in xflg
        if (op == NULL)
            continue;
        if (op[strlen(op)] == '*') {        // lp1 - test for *
            iflg++;                         // set iflg
            op[strlen(op)] = '\0';          // eliminate the *
        }
        if (strpbrk(op,"'+-()*/$") != NULL) // illegal character
            sm1++;                          // undefined command
        mne = srch(mt,op);                  // look up mnemonic
        if (*(int *)mne == (int)mne)
            popi();                         // undefined, must be pop
        word = mne->word;                   // instruction skeleton
        mode = mne->mode;                   // save op code type
        if ((mode & DIR1)) {                // 00000004
            dir(word & 07777);
            continue;
        }
        dlbl(lbl1.lbl,loc);                 // define label
        if ((mode & DIR2)) {                // lin7
            dir(word & 07777);
            continue;
        }
        if (pass2) {                    // build instruction on pass 2
            if (mode & 02000)
                popr();                 // :292
            else if (mode & 01)
                fref();                 // :294
            else if (mode & 010) {       // INS2 000000010 or INS9 00004010
                inr3();                 // :356
            }
            else {
                if (mode & INS1)            // 00000020
                    octf = FORCEO;
                else
                    octf = 0;
                word |= (scnl() & 037777);             // inr1 - get instruction address
                wmod = mode;
                // EOM or EOD
                if (iflg)
                    word |= 040000;         // inr92 - set indirect bit
                // inr94
                octf = 0;                  // reset octal flag
                if (term == ',') {
                    scan();                 // get index
                    word |= (valu & XMASK) << 21; // inr2  insert index
                }
                if ((term != ' ')  && (term != '\0'))       // lin8  test if blank or \0 terminator
                    eerr++;
                term = ' ';
                edit(044040000,word);       // edit instruction form
                outp(RECDAT,0,(tbl *)&word,loc);           // type 0 output item
            }
        }
        loc++;             // lin2 pass 1 and 2, inst and directive
    }
    return 1;
}


