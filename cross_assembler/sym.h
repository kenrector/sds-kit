//
//  sym.h
//
//  Created by Ken Rector on 3/15/20


#define INS1  020
#define INS9 04010
#define INS2 010
#define DIR1 04
#define DIR2 02

#define RELM 02
#define EXTM 01



#define REFT 01            // ref/def/pop subtype
#define DEFT 02
#define INTT 00

#define RECDAT 00            // record type
#define RECDEF 01
#define RECPOP 02
#define RECEND 03

#define AORG 0103
#define BCD  0104
#define BCI  0105
#define BORG  0106
#define BSS  0107
#define DEC  0110
#define DED  0111
#define END  0112
#define EQU  0113
#define FORM  0114
#define NOPO  0115
#define OCT  0116
#define OPD  0117
#define POPD 0120
#define PZE  0121
#define TEXT 0122
#define BOOL 0123
#define COPY  0124
#define DATA  0125
#define ORG  0126
#define PAGE 0127
#define BPT 0130

#define FORCEO 1
#define XMASK  07
    
#define EOM 00200000

typedef struct def {
    int     cw;
    char    lbl[8];
    int     data;
} def;



typedef struct tbl {
    char lbl[8];
    int  mode;
    int  word;
} tbl;


#define CTT(c)  ctt[ascii_to_sds930[c]]

#define IC  04000000
#define CC  02000000
#define LC  01000000
#define SC  00400000
#define DC  00200000
#define OC  00100000
#define XYZ 06000000


#define ITM 04000000           // ITEM
#define CON 02000000           // CONNECTOR
#define STM 05400000           // SYMBOL,LABEL,ITEM
#define DTM 04600000           // DECIMAL, SYMBOL, ITEM
#define OTM 04700000           // OCTAL,DECIMAL,SYMBOL,ITEM
#define SCO 0                   // ZERO
#define SCD 0                   // DIGIT
#define SCS 0                   // LETTER
#define SCA 1                   // APOSTROPHY  (single quote)
#define SCL 5                   // DOLLAR, ASTERISK
#define SCX 9                   // LEFT PAREN

#define BCON 00000                // BLANK connector code
#define CCON 00100                // , connector code
#define RCON 00200                // ) connector code
#define SCON 03310               // +  ++ connector codes
#define DCON 03411               // -  -- connector codes
#define PCON 04522               // *  ** connector codes
#define QCON 04647               // /  // connector codes



#define CLS 0
#define CLD 1
#define CLP 2
#define CAS 3
#define CAD 4
#define CAP 5
#define CEQ 6
#define CXQ 7
#define CDS 8
#define CBS 9
