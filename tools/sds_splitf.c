/* sds_splitf.c - split a file composed of several object files*/

#include <stdio.h>
#include <string.h>


FILE *infile, *outfile;
int btz;

#define eof 027657537

unsigned int get24(){
unsigned int i,j;

  i = (j = fgetc(infile)) & 077;
    fputc(j,outfile);
  i = i<<6 | ((j = fgetc(infile)) & 077);
    fputc(j,outfile);
  i = i<<6 | ((j = fgetc(infile)) & 077);
    fputc(j,outfile);
  i = i<<6 | ((j = fgetc(infile)) & 077);
    fputc(j,outfile);
    btz += 4;
  return(i);
}

int main(int argc, char *argv[])
{
  unsigned int i,j,n;
    unsigned int bin,type,cksm,cnt;
    unsigned int len;
    char fl[32];
    int  fn;

    btz = 0;

  if (argc != 3)
  {
    fprintf(stderr,"Split file composed of several object files\n");
    fprintf(stderr,"Usage: sds_splitf  binary \n");
    return 1;
  }

  infile = fopen(argv[1], "rb");
  if (!infile)
  {
    fprintf(stderr,"Cannot open binary file: %s\n", argv[1]);
    return 1;
  }
    sprintf(fl,"%s%0.3d",argv[2],fn++);
    outfile = fopen(fl,"w");
 
  do
  {
    i = get24();
    if (feof(infile))
      break;
   if (i!=eof)
   {
       bin = (i >> 12) & 07;
       if (bin != 05) {
           for (j = 1; j < 40; j++) {
               i = get24();
           }
           continue;
       }
       type = i >> 21;
       cnt = (i >> 15) & 077;
       cksm = i & 07777;
       printf("%08o    ",i);
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
       len = 40;
       if (type == 0 ) {
           i = get24();
           printf("%08o    load address = %08o\n",i,i);
           cnt--;
           len--;
       }
       for (j = 1; j<cnt;j++) {
           i = get24();
           //printf("%08o \n",i);
           
       }
       for (;j < len;j++)
           i = get24();
       
       if ((type == 3) & (fn < 3)){
           fclose(outfile);
           sprintf(fl,"%s%0.3d",argv[2],fn++);
           outfile = fopen(fl,"w");
       }
   }
    n++;
  } while(!feof(infile));

  fclose(infile);
  fclose(outfile);
}
