//
// sym2.c
//
//
//  Created by Ken Rector on 3/21/20.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#include "sym.h"

extern tbl *srch(tbl *t, char *key);
extern void _move(tbl *item, char *key, int loc, int mode);

extern char *var;              // operand string
extern char refr[];
extern int *lop;
extern int *litm;
extern int ladd;
extern int *scnx;


extern int derr;
extern int eerr;
extern int iflg;
extern int loc;
extern int lsot;
extern int lsxt;
extern int mode;
extern int octf;
extern int pass2;
extern int perr;
extern int relm;
extern int rerr;
extern int scno;
extern int term;
extern int terr;
extern int _verr;
extern int valu;
extern int xerr;

void scan(void);



tbl mt[1000] = {
    {"LCY", 00, 06720000},
    {"LDA", 00, 07600000},
    {"LDB", 00, 07500000},
    {"LDE", 00, 04600140},
    {"LDX", 00, 07100000},
    {"LIL", DIR2, NOPO},
    {"LIST", DIR2, NOPO},
    {"LRSH", 00, 06624000},
    {"LSH", 00, 06700000},
    {"MIN", 00, 06100000},
    {"MIW", 00, 01200000},
    {"MIY", 00, 01000000},
    {"MPT", INS9, 010210},
    {"MRG", 00, 01600000},
    {"MUL", 00, 06400000},
    {"NOD", 00, 06710000},
    {"NOP", 00, 02000000},
    {"OCT", DIR2, OCT},
    {"OPD", DIR1, OPD},
    {"ORG",  DIR1, ORG},
    {"OVT",  00, 04020001},
    {"PAGE", DIR2, PAGE},
    {"PBF", INS9, 017457+EOM},
    {"PBT", INS9, 04012045},
    {"PCB", INS9, 03045+EOM},
    {"PCD", INS9, 02045+EOM},
    {"PFT", INS9, 011057},
    {"PLP", INS9, 02057+EOM},
    {"POL", INS9, 012057+EOM},
    {"POPD", DIR1, POPD},
    {"POT",  00, 01300000},
    {"PPT",  INS9, 02043+EOM},
    {"PPTW", INS2, 0202043},
    {"PPTY", INS2, 0202143},
    {"PRT",  INS9, 012057},
    {"PS1F", INS9, 010457+EOM},
    {"PS2F", INS9, 012457+EOM},
    {"PSP1", INS9, 011657+EOM},
    {"PSP2", INS9, 012657+EOM},
    {"PSP3", INS9, 013657+EOM},
    {"PSP4", INS9, 014657+EOM},
    {"PSP5", INS9, 015657+EOM},
    {"PSP6", INS9, 016657+EOM},
    {"PSP7", INS9, 017657+EOM},
    {"PTF", INS9, 011457+EOM},
    {"PTL", INS9, 043+EOM},
    {"PTLW", INS2, 0200043},
    {"PTLY", INS2, 0200143},
    {"PZE", DIR2, PZE},
    {"RCB", INS9, 03005+EOM},
    {"RCBW", INS2, 0203005},
    {"RCBY", INS2, 0203105},
    {"RCD", INS9, 02005+EOM},
    {"RCDW", INS2, 0202005},
    {"RCDY", INS2, 0202105},
    {"RCH", INS1, 04600000},
    {"RCY", 00, 06620000},
    {"REL", DIR2, NOPO},
    {"REO", 00, 020010+EOM},
    {"RES", DIR2, BSS},
    {"REW", INS9, 014010+EOM},
    {"REWW", INS2, 014010+EOM},
    {"RKB", INS9, 02000+EOM},
    {"RKBW", INS2, 0202000},
    {"RKBY", INS2, 0202100},
    {"RORG", DIR2, ORG},
    {"ROV", 00, 020001+EOM},
    {"RPT", INS9, 02003+EOM},
    {"RPTW", INS2, 0202003},
    {"RPTY", INS2, 0202103},
    {"RSH", 00, 06600000},
    {"RTB", INS9, 03010+EOM},
    {"RTBW", INS2, 0203010},
    {"RTBY", INS2, 0203110},
    {"RTD", INS9, 02010+EOM},
    {"RTDW", INS2, 0202010},
    {"RTDY", INS2, 0202110},
    {"RTS", INS9, 014000+EOM},
    {"SFB", INS9, 03030+EOM},
    {"SFB", INS2, 0203030},
    {"SFD", INS9, 02030+EOM},
    {"SKA", 00, 07200000},
    {"SKB", 00, 05200000},
    {"SKD", 00, 07400000},
    {"SKE", 00, 05000000},
    {"SKG", 00, 07300000},
    {"SKM", 00, 07000000},
    {"SKN", 00, 05300000},
    {"SKR", 00, 06000000},
    {"SKS", INS1, 04000000},
    {"SRB", INS9, 07030+EOM},
    {"SRBW", INS2, 0207030},
    {"SRC", INS9, 012005+EOM},
    {"SRD", INS9, 06030+EOM},
    {"SRR", INS9, 013610+EOM},
    {"STA", 00, 03500000},
    {"STB", 00, 03600000},
    {"STE", 00, 04600122},
    {"STX", 00, 03700000},
    {"SUB", 00, 05400000},
    {"SUC", 00, 05600000},
    {"TCD", DIR2, NOPO},
    {"TEXT", DIR2, TEXT},
    {"TFT", INS9, 013610},
    {"TGT", INS9, 012610},
    {"TOP", INS9, 014000+EOM},
    {"TOPW", 00, 014000+EOM},
    {"TOPY", 00, 014100+EOM},
    {"TRT", INS9, 04010410},
    {"TYP", INS9, 02040+EOM},
    {"TYPW", INS2, 0202040},
    {"TYPY", INS2, 0202140},
    {"UNLI", 0, 0},
    {"WIM", 00, 03200000},
    {"WTB", INS9, 03050+EOM},
    {"WTBW", INS2, 0203050},
    {"WTBY", INS2, 0203150},
    {"WTD", INS9, 02050+EOM},
    {"WTDW", INS2, 0202050},
    {"WTDY", INS2, 0202150},
    {"XAB", 00, 04600014},
    {"XEE", 00, 04600160},
    {"XMA", 00, 06200000},
    {"XXA", 00, 04600600},
    {"XXB", 00, 04600060},
    {"YIM", 00, 03000000},
    {"ABC", 00, 04620005},
    {"ADC", 00, 05700000},
    {"ADD", 00, 05500000},
    {"ADM", 00, 06300000},
    {"AIR", 00, 020020+EOM},
    {"ALC", INS9, 0250000},
    {"AORG", DIR1, AORG},
    {"ASC", INS9, 012000+EOM},
    {"BAC", 00, 04610012},
    {"BCD", DIR2, BCD},
    {"BCI", DIR2, BCI},
    {"BETW", 00, 04020010},
    {"BETY", 00, 04020020},
    {"BLK", DIR2, NOPO},
    {"BOOL", DIR1, BOOL},
    {"BORG", DIR1, BORG},
    {"BPT", DIR2, BPT},
    {"BRM", 00, 04300000},
    {"BRR", 00, 05100000},
    {"BRTW", 00, 04021000},
    {"BRTY", 00, 04022000},
    {"BRU", 00, 00100000,},
    {"BRX", 00, 04100000},
    {"BSS", DIR2, BSS},
    {"BTT", INS9, 04012010},
    {"CAB", 00, 04600004},
    {"CAT", INS9, 04014000},
    {"CAX", 00, 04600400},
    {"CBA", 00, 04600010},
    {"CBX", 00, 04600020},
    {"CET", INS9, 04011000},
    {"CFT", INS9, 04011005},
    {"CIT", INS9, 04010400},
    {"CLA", 00, 04600001},
    {"CLB", 00, 04600002},
    {"CLR", 00, 04630003},
    {"CLX", 00, 024600000},
    {"CNA", 00, 04601000},
    {"COPY", DIR2, COPY},
    {"COPY", 0, 0},
    {"CPT", INS9, 04014045},
    {"CRT", INS9, 04012005},
    {"CXA", 00, 04600200},
    {"CXB", 00, 04600040},
    {"CZT", INS9, 04012000},
    {"DATA", DIR2, DATA},
    {"DEC", DIR2, DEC},
    {"DED", DIR2, DED},
    {"DIR", 00, 00220004},
    {"DISW", 00, 00+EOM},
    {"DISY", 00, 0100+EOM},
    {"DIV", 00, 06500000},
    {"DSC", INS9, 00+EOM},
    {"DT2", INS9, 016210},
    {"DT5", INS9, 016610},
    {"DT8", INS9, 017210},
    {"EAX", 00, 07700000},
    {"EFT", INS9, 03070+EOM},
    {"EIR", 00, 00220002},
    {"END", DIR2, END},
    {"EOD", INS1, 0600000},
    {"EOM", INS1, 0200000},
    {"EOR", 00, 01700000},
    {"EPT", INS9, 014057},
    {"EQU", DIR1, EQU},
    {"ERT", INS9, 07070+EOM},
    {"ETR", 00, 01400000},
    {"ETT", INS9, 04011010},
    {"ETW", INS2, 0203070},
    {"EXU", 00, 02300000},
    {"F101", 0, 0},
    {"FCT", INS9, 04014005},
    {"FORM", DIR1, FORM},
    {"FORT", DIR2, NOPO},
    {"HLT", 0, 0},
    {"FPT", INS9, 04014010},
    {"IDT", 00, 04020002},
    {"IET", 00, 04020004},
    {"QQQ",00, 0000},
    {"      ", 00,00000}
};


tbl sym[1000] = {{"\0\0\0", 00,00000}               // symbol table
};
tbl lt[1000] = {{"\0\0\0", 00,00000}                // literal table
};
tbl rt[1000] = {{"\0\0\0", 00,00000}                // ref table
};


const uint8_t ascii_to_sds930[128] = {
    060,  -1,  -1,  -1,  -1,  -1,  -1,  -1,             /* 00 - 37 */
    032, 072,  -1,  -1,  -1, 052,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    060, 052,  -1, 077, 053, 017,  -1, 014,             /* 40 - 77 */
    074, 034, 054, 020, 073, 040, 033, 061,
    000, 001, 002, 003, 004, 005, 006, 007,
    010, 011, 015, 056, 036, 013, 016, 072,
    037, 021, 022, 023, 024, 025, 026, 027,             /* 100 - 137 */
    030, 031, 041, 042, 043, 044, 045, 046,
    047, 050, 051, 062, 063, 064, 065, 066,
    067, 070, 071, 035, 076, 055, 057, 060,
    -1, 021, 022, 023, 024, 025, 026, 027,             /* 140 - 177 */
    030, 031, 041, 042, 043, 044, 045, 046,             /* fold lower case to upper */
    047, 050, 051, 062, 063, 064, 065, 066,
    067, 070, 071,  -1,  -1,  -1,  -1,  -1
};


int ctt[] = {
    OTM |SCO,         // O OO
    OTM |SCD,         // 1 01
    OTM |SCD,         // 2 02
    OTM |SCD,         // 3 03
    OTM |SCD,         // 4 04
    OTM |SCD,         // 5 05
    OTM |SCD,         // 6 06
    OTM |SCD,         // 7 07
    DTM |SCD,         // 8 10
    DTM |SCD,         // 9 11
    0,                // Z HLT 0
    077,                // C3 HLT 077 =
    ITM |SCA,         // ' 14
    040000000,                // 80 DATA 040000000
    2,                // 822 HLT 2
    030,                // P24 HLT 24
    CON |SCON,        // + 20
    STM |SCS,         // A 21
    STM |SCS,         // B 22
    STM |SCS,         // C 23
    STM |SCS,         // D 24
    STM |SCS,         // E 25
    STM |SCS,         // F 26
    STM |SCS,         // G 27
    STM |SCS,         // H 30
    STM |SCS,         // I 31
    013,                // ECHR |HLT '='
    040,                // MCHR |HLT '-'
    CON |RCON,        // ) 34
    07,                // A7 HLT 7
    03,                // P3 HLT 3
    0100,                // B17 HLT 0100
    CON |DCON,        // - 40
    STM |SCS,         // J 41
    STM |SCS,         // K 42
    STM |SCS,         // L 43
    STM |SCS,         // M 44
    STM| SCS,         // N 45
    STM |SCS,         // O 46
    STM |SCS,         // P 47
    STM |SCS,         // Q 50
    STM| SCS,         // R 51
    020,                // PLUS HLT '+'         // +
    ITM |SCL,         // $ 53
    XYZ |SCL,         // * 54
    033,                // PCHR HLT '.'
    014,                // QCHR HLT 014
    034,                // RCHR HLT ')'
    CON |BCON,        // 60   ' '    = 02000000
    CON |QCON,        // / 61
    STM |SCS,         // S 62
    STM |SCS,         // T 63
    STM |SCS,         // U 64
    STM |SCS,         // V 65
    STM |SCS,         // W 66
    STM |SCS,         // X 67
    STM |SCS,         // Y 70
    STM |SCS,         // Z 71
    0,                // HLT 0
    CON |CCON,        // , 73
    ITM |SCX,         // ( 74
    022,                // HB HLT  'B'
    025,                //  HE HLT  'E'
    020000000                 // X2W HLT 0,X2
};

int chr1;



// evaluate a connector                     partII 660
char * scc(char *s, int *m) {
    //int flg;
    int cnct;
    
    if (!(CTT(*s) & CC)) {
        term = *s;                    // not a connector
        eerr++;                         // set E flag -  skip char
        *m = mode;
        return ++s;                     // skip char
    }
    if (*s == '*') {                  // scc1 conecter is AP if *
        chr1 = *s;                    // save it
        s++;
        if (*s == '+') {
            cnct = 05000;               // *+    decimal scale connector
            // X *+ Y -> (X).(10^Y)                    //
        }
        else if (*s == '/') {              // */ binary scale connector
            cnct = 05100;               // X */ Y -> (X).(2^Y)
            // scc4  position at next char
        }
        else {
            cnct = PCON;                // PCON = 04522
            if (*s == '*') {
                cnct = cnct << 6;       // ** == 2200 code for pair
            }
        }
    }
    else {
        cnct = CTT(*s);                 // scc7 - save conecter
        if (!(cnct & 07000)) {          // if ), ,, or ' '
            term = *s;                  // save terminater
        }
        else {
            chr1 = *s;        // scc2  not a terminator - save char
            if (s[1] == chr1) {          // if paired
                cnct = cnct << 6;        // use alternate connector
                s++;
            }
        }
    }
    *m = mode | (cnct & 07700);          // scc3 extract connecter
    return ++s;
}


// evaluates an item   part II - 431
char *sci(char *s, tbl *item) {
    int i;
    int n;
    char itm[12];
    tbl *t;
    
    while (1) {
        if (!(CTT(*s) & IC)) {     // & 04000000 - OTM or DTM or STM  symbol worthy
            eerr++;
            s++;
            return s;
        }
        switch(CTT(*s) & 07777) {
            case SCO:                       // digits  and alpha
                //chr2 = *s;                // save lead char
                if (!(CTT(*s) & SC)) {     // if 1st char not item character
                    strncpy(item->lbl,"      ",6);   // return error
                    return s;
                }
                n = (int)strlen(s);
                for (i = 0; i < n; i++) {
                    if (!(CTT(*s) & SC))      // if not item character
                        break;
                    itm[i] = *s;
                    s++;
                }
                itm[i] = '\0';
                n = 0;
                for (i = 0; i < strlen(itm);i++) {
                    if (!isdigit(itm[i]))
                        break;
                    if ((itm[i] == '8') || (itm[i] == '9'))
                        n++;
                }
                if (i == strlen(itm)) {
                    if ((itm[0] == '0') || octf) {
                        if (!n)
                            n = sscanf(itm,"%o",&item->word);
                        else
                            n = 0;          // found decimal digits
                    }
                    else
                        n = sscanf(itm,"%d",&item->word);
                    if (!n)
                        eerr++;
                    item->mode = 0;
                }
                else {
                    t = srch(sym,itm);      // scit4  search a= valu  b= mode
                    strncpy(item->lbl,itm,6);
                    if (*(int *)t == (int)t) {
                        _verr++;            // 543 undefined referenced
                    }
                        if ((t->mode & 0100))
                            derr++;         // refers to duplicate
                        item->mode = t->mode & 03;
                        if (t->word & 0100000)
                            item->mode |= RELM;
                        item->word = t->word;
                        if (item->mode  & RELM)
                            item->word &= 037777; // clean up address
                }
                return (s);
            case SCA:                       // 602  apostrophe (single quote)
                valu = 0;                   // sca2
                s++;                        // skip the quote
                while (*s != '\'') {        // scan for closing quote
                    if (valu & 077000000) { // sca1  alf char
                        terr++;             // more than four characters
                    }
                    else {
                        valu = (valu << 6) | ascii_to_sds930[*s];  // insert new char
                    }
                    s++;
                }
                s++;                        // sca3 skip second quote
                item->word = valu;
                item->mode = 0;
                return s;
            case SCL:                       // 555  * or $
                if (*s == '*') {            // test for *
                    s++;
                    if (!(CTT(*s) & CC)) {
                        iflg++;             // next character is a connector
                        continue;
                    }
                }
                else
                    s++;                    // scl1  skip $
                if (loc & 00100000)           // scl2  if relocatable
                    item->mode = RELM;
                else
                    item->mode = 0;
                item->word = loc & 037777;
                return s;
                
            case SCX:                   // 621 (
                /*
                if ((int)scnx <= scno) {
                    perr++;             // scx1  too many levels - set p flag
                    s++;
                    while (*s != ')')
                        s++;            // skip sub-expression
                    s++;                // skip )
                    item->word = 0;
                    item->mode = 0;
                    return s;
                }
                 */
                scnx++;
                lop++;
                litm++;
                var++;                    // skip (
                scan();                 // get sub expression
                lop--;
                litm--;
                scnx--;
                item->word = valu;
                item->mode = 0;
                return var;
        }
    }
    return s;
}

// evaluate symbolic expression               part II 315:
// returns with valu and mode for address field
//         and  term == last terminator character
void scan() {
    
    int i;
    int n;
    int B;
    
    
    tbl t;
    *lop = 0;                   // 0 to base conecter
    valu = 0;                   // 0 to value
    mode = 0;                    // 0 to mode
    chr1 = 0;                       // reset

    if (CTT(*var) & IC) {    // & IC 04000000   connector flag?
        var = sci(var,&t);   // scn1: not a connecter - get next item
        strncpy(refr,t.lbl,6);    // save ref label
        valu = t.word;
        mode = t.mode;      // mode indicates RELM or zero
    }
    while (1) {
        // evaluate next connector
        // returns the connector code from CTT[*var] or
        // a generated code for two character connectors
        // masked with 07700
        var = scc(var,&mode);    // scn8  get next connecter

    scn6:
        if ((mode <= *lop) ||
            ((mode & 07000) == *lop)) {  // scn7 hierarchy mask
            if ((*lop & 07000) == 0) {       // scn2  last connector - test for terminator
                B = mode & 077;             // mask mode  - gives connector code
                if ((int)scnx == lsxt) {     // test p level
                    if (term != ')') {       // p level zero
                        while (1) {               // scn12 - gnf()  skip to next field
                            if ((term == ',') ||
                                (term == ' ') ||
                                (term =='\0'))
                                return;
                            var++;
                            term = *var;
                        }
                    }
                    else {
                        perr++;             // level 0 and terminator == )
                    }
                }
                else {                      // not level 0
                    if (term != ')')            // scn11
                        perr++;
                }
                return;
            }
            else {
                i = *lop;                   // scn4  do operation
                if ((*lop & 07000) != 05000)
                    i &= 0700;
                i &= 01700;
                i = i >> 6;                  // op number
                switch (i) {                 // scn4 - do operation
                    case CLS:               // logical sum  V=L++V
                        valu = *litm | valu;
                        break;
                    case CLD:               // logical diff  V=L--V
                        valu = *litm ^ valu;
                        break;
                    case CLP:               // logical product V=L**V
                        valu = *litm & valu;
                        break;
                    case CAS:               // arithmetic sum  V=L+V
                        valu = *litm + valu;
                        break;
                    case CAD:               // arithmetic diff V=L-V
                        valu = *litm - valu;
                         break;
                    case CAP:               // arithmetic product  V=L*V
                        valu = *litm * valu;
                        break;
                    case CEQ:               // inclusive quotent V=L+V-1
                        valu = (*litm + valu - 1) / valu;
                         break;
                    case CXQ:               // exclusive quotent V=L/V
                        valu = *litm / valu;
                         break;
                    case CDS:               // decimal shift
                        if ((n = valu) < 0)
                            n  = abs(valu);
                        if (n > 9)
                            terr++;
                        valu = *litm * pow(10,n);
                    case CBS:               // binary shift
                        n = valu;
                        valu = *litm * pow(2,n);
                        break;              // goto CAP1
                }
                switch (i) {
                    case CEQ:               // inclusive quotent
                    case CXQ:               // exclusive quotent
                    case CLS:               // logical sum
                    case CLD:               // logical difference{
                    case CLP:               // logical product
                    case CAP:               // arithmetic product
                    case CDS:
                    case CBS:
                        if ((mode | *lop) & RELM)  // cls1  error if either are relative
                            rerr++;
                        break;
                    case CAS:               // arithmetic sum
                        if ((mode & *lop) & RELM)  // cls2  error if both are relative
                            rerr++;
                        break;
                    case CAD:               // arithmetic difference
                        if ((mode & (*lop ^ -1)) & RELM)
                            rerr++;
                        break;
                }
                // if previous mode was absolute, invert mode
                if ((*lop & RELM))
                    mode = mode ^ RELM;

                lop--;              // scn6
                litm--;
                goto scn6;          // avoided another loop
            }
        }
        else {
            *(++lop) = mode;            // scn3  store op
            *(++litm) = valu;            // store item
            var = sci(var,&t);          // get next item
            valu = t.word;
            mode = t.mode;
        }
    }
    return;
}

// insert literal
int scnm(char *s) {
    tbl *t;
    if (!pass2)
        return 0;
    t = srch(lt,s);                 // search for literal
    if (*(int *)t == (int)t) {
        _move(t,s,ladd,mode);          // not found, insert literal in table
        mode = ladd & 0100000 ? RELM : 0;
        valu =ladd;
        ladd++;                     // increment literal location
    }
    else {
        mode = t->word & 0100000 ? RELM : 0; // sck3 found it
        valu = t->word & 037777;
    }
    return valu;
}

// scan reference
// if just one symbol - its external ref
int scnr(int conn) {
    tbl *t;
    //char *s;
    //int tmp;
    
    scan();                         // get expression
    if (_verr <= 0)                 // undefined flag
        return valu;                // all symbol defined - no ref or literal
    if (chr1 != 0)                  // test for no connectors
        return valu;                // undefined item
    if (pass2) {
        _verr = 0;                   // reset undefined flag
        xerr++;                      // set external flag
        t = srch(rt,refr);           // search for reference
        if (*(int *)t == (int)t) {
            _move(t,refr,loc,0);     // scr4  insert reference
            valu = 0;                // no previous ref
            mode = 0;
        }
        else {
            valu = t->word;          // scr4  found it
            t->word = loc;           // link to next in chain
            t->mode = (loc & 0100000) ? 0202 : 0200;
            mode = t->mode;
        }
        return valu;
    }
    else
        return 0;
}

// scan address field
int  scnl() {
    //tbl v;
    int i = 0;
    //char *s;
    
    if (*var != '=')              // test for =
        i = scnr(i);                  // not literal - get expression
    else {
        var++;                      // skip =
        scan();
        i = scnm((char *)&valu);
    }
    return i;
}
