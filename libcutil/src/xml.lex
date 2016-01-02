%{
# include "mxml.hpp"
# include "xml.h"
# include <stdio.h>
  //# include <io.h>
# include <string.h>
  extern int linenum;
#ifdef WIN
extern "C"
{
  extern char *strdup(const char *);
}
#endif

using namespace utils;
using namespace utils::model;

//extern int fileno(FILE *);

#define YY_NEVER_INTERACTIVE 1

int count_lines(const char *s);
//#ifdef LINUX
//extern int yywrap(void);
//#endif

// Bug list: parser hang if "'" character found
// \'([^\']|(\\\'))*\' {xmllval.string = strdup(yytext); return CHAINE;}
//\"\" {xmllval.string = strdup(yytext); return CHAINE;}
//\"[^\"]*\" {xmllval.string = strdup(yytext); return CHAINE;}
%}

%%

[ \t]  return ESP;
"\n" {linenum++; return LF;}
"<!--"([^-]|("-"[^-])|("--"[^>]))*"-->" {linenum += count_lines(yytext);};
"\"" return GUILL;
"'" return APOS;
"?"        return INTEROG;
"--" {return MM;}
"!" {xmllval.string = strdup(yytext); return EXCLA;}
"<"    return INF;
">"    return SUP;
"="      return EGAL;
"/"       return SLASH;
[$\-0-9a-zA-Z_îéèêôùàç.:;,*+!\|\(\)&{}%\\#@\[\]`~°â^öÖäüßÄÜ]* {xmllval.string = strdup(yytext); return WWORD;}
%%

// Iso
// [\-0-9a-zA-Z_ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½.:;,*+!\|\(\)&{}%\\#]* {xmllval.string = strdup(yytext); return WWORD;}

//[^;]/;   return UNTILSEMICOLON;


void yyerror (char *fmt, ...)
{
}

int count_lines(const char *s)
{
    int res = 0;
    for(int i = 0; i < (int) strlen(s); i++)
    {
        if(s[i] == '\n')
            res++;
    }
    return res;
}  
