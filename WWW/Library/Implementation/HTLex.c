
/* MODULE							HTLex.c
**		LEXICAL ANALYSOR
**
** AUTHORS:
**	AL	Ari Luotonen	luotonen@dxcern.cern.ch
**
** HISTORY:
**
**
** BUGS:
**
**
*/

#include <HTUtils.h>
#include <HTAAUtil.h>
#include <HTLex.h>	/* Implemented here */

#include <LYLeaks.h>

/*
** Global variables
*/
PUBLIC char HTlex_buffer[40];	/* Read lexical string		*/
PUBLIC int HTlex_line = 1;	/* Line number in source file	*/


/*
** Module-wide variables
*/
PRIVATE int lex_cnt;
PRIVATE BOOL lex_template;
PRIVATE LexItem lex_pushed_back = LEX_NONE;
PRIVATE FILE *cache = NULL;


PUBLIC void unlex ARGS1(LexItem, lex_item)
{
    lex_pushed_back = lex_item;
}


PUBLIC LexItem lex ARGS1(FILE *, fp)
{
    int ch;

    if (fp != cache) {	/* This cache doesn't work ok because the system  */
	cache = fp;	/* often assign same FILE structure the next open */
	HTlex_line = 1;	/* file. So, if there are syntax errors in setup  */
    }			/* files it may confuse things later on.	  */

    if (lex_pushed_back != LEX_NONE) {
	LexItem ret = lex_pushed_back;
	lex_pushed_back = LEX_NONE;
	return ret;
    }

    lex_cnt = 0;
    lex_template = NO;

    for(;;) {
	switch (ch = getc(fp)) {
	  case EOF:
	  case ' ':
	  case '\t':
	  case '\r':
	  case '\n':
	  case ':':
	  case ',':
	  case '(':
	  case ')':
	  case '@':
	    if (lex_cnt > 0) {
		if (ch != EOF) ungetc(ch,fp);
		if (lex_template) return LEX_TMPL_STR;
		else		  return LEX_ALPH_STR;
	    }
	    else switch(ch) {
	      case EOF:		return LEX_EOF;		break;
	      case '\n':
		HTlex_line++;	return LEX_REC_SEP;	break;
	      case ':':		return LEX_FIELD_SEP;	break;
	      case ',':		return LEX_ITEM_SEP;	break;
	      case '(':		return LEX_OPEN_PAREN;	break;
	      case ')':		return LEX_CLOSE_PAREN;	break;
	      case '@':		return LEX_AT_SIGN;	break;
	      default:	;	/* Leading white space ignored (SP,TAB,CR) */
	    }
	    break;
	  default:
	    HTlex_buffer[lex_cnt++] = ch;
	    HTlex_buffer[lex_cnt] = '\0';
	    if ('*' == ch) lex_template = YES;
	} /* switch ch */
    } /* forever */
}


PUBLIC char *lex_verbose ARGS1(LexItem, lex_item)
{
    static char msg[100];

    switch (lex_item) {
      case LEX_NONE:		/* Internally used	*/
	return "NO-LEX-ITEM";
	break;
      case LEX_EOF:		/* End of file		*/
	return "end-of-file";
	break;
      case LEX_REC_SEP:		/* Record separator	*/
	return "record separator (newline)";
	break;
      case LEX_FIELD_SEP:	/* Field separator	*/
	return "field separator ':'";
	break;
      case LEX_ITEM_SEP:	/* List item separator	*/
	return "item separator ','";
	break;
      case LEX_OPEN_PAREN:	/* Group start tag	*/
	return "'('";
	break;
      case LEX_CLOSE_PAREN:	/* Group end tag	*/
	return "')'";
	break;
      case LEX_AT_SIGN:		/* Address qualifier	*/
	return "address qualifier '@'";
	break;
      case LEX_ALPH_STR:	/* Alphanumeric string	*/
	sprintf(msg, "alphanumeric string '%s'", HTlex_buffer);
	return msg;
	break;
      case LEX_TMPL_STR:	/* Template string	*/
	sprintf(msg, "template string '%s'", HTlex_buffer);
	return msg;
	break;
      default:
	return "UNKNOWN-LEX-ITEM";
	break;
    }
}

