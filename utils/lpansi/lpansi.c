/* This program allows printing of files from any VT100 or ANSI by simply 
   calling the program name and a filename argument.
   Written by: Gary Day, 11/30/93
   Hey, it doesn't have to be complicated to be useful.
*/

/* Ansi C function prototypes */
void ansi_printer_on(void);
void ansi_printer_off(void);
int main(int argc, char *argv[]);

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  int  ch;    /* Where we store our characters */
  FILE *fp;   /* File pointer */
  if (argc!=2)
   {
    printf("Usage is lpansi <filename>\n");
    exit(1);
   }
/* If you got here, then there is only 1 filename argument - correct */
  if ((fp=fopen(argv[1], "r"))==NULL)
  {
   printf("Can't open %s\n",argv[1]);
   exit(1);
  }
/* Ok, the filename was there, lets do it! */

  ansi_printer_on();

  while ((ch=getc(fp))!=EOF)
  {
   putc(ch,stdout);
  }
  fclose(fp);
  
  printf("\n\x0C");  /* Do a form feed at the end of the document */

  ansi_printer_off();

  return 0;  /* Return a zero if everything is ok */
} 

/* Send a printer on escape sequence */
void ansi_printer_on(void)
{
  printf("\x1B[5i");
}

/* Send a printer off escape sequence */
void ansi_printer_off(void)
{
  printf("\x1B[4i");
}
