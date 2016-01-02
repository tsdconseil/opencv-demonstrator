/** C++ grammar and rules for xml parsing **/

/** Declaration zone
 *  (C part : includes, defines, and prototypes )
 *  This part will not be included in the file "y.tab.h" !
**/
%{
#include "cutil.hpp"
#include "mxml.hpp"
#include <vector>
#include <string.h>
#include <cstdio>


using namespace utils;
using namespace utils::model;

#ifdef WIN
extern "C"
{
  extern char *strdup(const char *);
}
#endif

extern void yyerror(const char *s);
extern int yylex();
MXml *closure;


  int linenum = 1;            /* current line            */
  int indent = 0;             /* current indentation     */
  extern std::string current_file;  /* current file to be read */
  extern char **result;       /* the buffer where to put the result */
  int last_lex;

  /** Get the last lexical element .
   *  (for parse errors management purpose)
  **/
  char *strconc(char *s1, char *s2);

%}

/** YACC Declarative Part
 *  This part will be included into the file "y.tab.h".
 *  Contains:
 *   - Typification of the synthetized attributes
 *     issued from lexical analysis (uppercase),
 *     and syntaxic analysis (lowercase);
 *   - Some priority rules to avoid reduce/reduce conflicts.
**/

/* All possible synthetized attributes types */
%union
{
  char *string;
  utils::model::MXml *mxml;
  utils::model::XmlAttribute *attribute;
  std::vector<utils::model::XmlAttribute> *attributes;
  std::vector<utils::model::MXml> *mxmls;
}



/* Tokens : terminal symbols issued from lexical analysis */



%token INF SUP SLASH EXCLA MM INTEROG EGAL GUILL APOS ESP LF
%token<string> WWORD CHAINE


%type <mxml>   balise
%type <mxml> balises balises_st balises_tx
%type <attribute> attribute
%type <attributes> attributes
%type <string> chaine chaine_guill_content chaine_guill_element chaine_apos_content chaine_apos_element texte_elt

 
/* start non-terminal symbol */
%start filefile


/** rules part
**/
%%

filefile: {linenum = 1;} file;


/** File synthetize only a raw string
 *  (not used)
**/
file
: esps entry esps comment_ob esps balise esps
{
  //printf("Fichier xml comm.\n"); fflush(0);
  closure = $6;
  //printf("done file.\n"); fflush(0);
}
| esps entry esps balise esps
{
  //printf("Fichier xml simple.\n"); fflush(0);
  closure = $4;
  //printf("done file.\n"); fflush(0);
}
| esps entry esps entry esps balise esps
{
  //printf("Fichier xml 2 entry.\n"); fflush(0);
  closure = $6;
  //printf("done file.\n"); fflush(0);
}
| error
{
    //printf("error.\n"); fflush(0);
  char buf[500];
  sprintf (buf, "Parse error at line %d, at token %s\n", 
    linenum, 
    "d"/*get_last_lex()*/);
  fprintf(stderr, "%s", buf);
  fflush(stderr);
  char buf2[500];
  sprintf(buf2, "Parse error in %s", current_file.c_str());
  //throw CException(std::string(buf2), std::string(buf));
  closure = NULL;
}
;


entry: INF INTEROG WWORD attributes INTEROG SUP
{
  free($3);
  delete $4;
};

comment_ob: INF EXCLA MM words MM SUP;

words : | WWORD esps words
{
  free($1);
};

balise: 
    INF WWORD attributes SLASH SUP
{
    $$ = new MXml(std::string($2), $3, new std::vector<MXml>);
    free($2);
}
|   INF WWORD attributes SUP balises INF SLASH WWORD SUP
{
	//printf("Building complex object...\n"); fflush(0);
    $$ = $5;
    $$->name = std::string($2);
    $$->attributes = *$3;
    delete $3;
    free($2);
    free($8);
	//printf("Done build complex object.\n"); fflush(0);
};


balises: 
{
    $$ = new MXml(); 
}
| balises_st
{
    $$ = $1;
}
| balises_tx
{
    $$ = $1;
};
// balise texte_ne balise          = balises(balises_tx(balise_st(balises(empty) balise) texte_ne))
// texte_ne balise texte_ne balise = NON ! pas possible commencer par texte !
//                                   balises_st(balises(balises_tx(texte_ne)))
/** Balises et textes avec texte � la fin */
balises_tx: balises texte_elt
{
    $$ = $1;
    $$->add_text(std::string($2));
    free($2);
};

/** Balises et textes avec balise � la fin */
balises_st: balises balise 
{
    $$ = $1;
    $$->add_child(*$2);
    delete $2;
};


attributes: esps
{
    $$ = new std::vector<XmlAttribute>;
}
| attributes attribute esps
{
    $$ = $1;
    $$->push_back(*($2));
    delete $2;
};

attribute: WWORD esps EGAL esps chaine
{
    $$ = new XmlAttribute($1, std::string($5));
    free($1);
    free($5);
};

chaine: GUILL chaine_guill_content GUILL
{
  $$ = $2;
}
| APOS  chaine_apos_content APOS
{
  $$ = $2;
};

chaine_guill_content: 
{
    $$ = strdup("");
}
| chaine_guill_content chaine_guill_element
{
    $$ = strconc($1, $2);
    free($1);
    free($2);
};

chaine_apos_content: 
{
    $$ = strdup("");
}
| chaine_apos_content chaine_apos_element
{
    $$ = strconc($1, $2);
    free($1);
    free($2);
};

chaine_guill_element: 
ESP {$$ = strdup(" ");} |
WWORD {$$ = $1;}
| APOS {$$ = strdup("'");}
| SLASH {$$ = strdup("/");}
| EGAL {$$ = strdup("=");}
| INTEROG {$$ = strdup("?");}
;

chaine_apos_element: 
ESP {$$ = strdup(" ");}
| WWORD {$$ = $1;}
| GUILL {$$ = strdup("\"");}
| SLASH {$$ = strdup("/");};

texte_elt: 
WWORD 
{
    $$ = $1;
}
| ESP
{
    $$ = strdup(" ");
}
| LF
{
    $$ = strdup("\n");
}
| APOS {$$ = strdup("'");}
| GUILL {$$ = strdup("\"");}
| SUP {$$ = strdup(">");}
| EGAL {$$ = strdup("=");}
| INTEROG {$$ = strdup("?");}
| EXCLA {$$ = strdup("!");}
| SLASH {$$ = strdup("/");};

esps:  
| ESP esps
| LF  esps;


%%
/**
 * functions zone
**/



extern "C" int xmlwrap(void)
{
  return 1;
} 

void xmlerror(const char *s) 
{
  printf("XML parse error: '%s'. line = %d\n", s, linenum);
}

char *strconc(char *s1, char *s2)
{
    char *res;
    res = (char *) malloc(strlen(s1)+strlen(s2)+1);
    int i;
    for(i = 0; i < (int) strlen(s1); i++)
        res[i] = s1[i];
    for(i = 0; i < (int) strlen(s2); i++)
        res[strlen(s1)+i] = s2[i];
    res[strlen(s1)+strlen(s2)] = 0;
    return res;
}




