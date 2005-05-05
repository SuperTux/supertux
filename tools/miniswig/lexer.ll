%{
#include <math.h>
#include <stdlib.h>
#include "tree.h"
#include "parser.hpp"
#include "globals.h"

#define YY_DECL int yylex YY_PROTO(( YYSTYPE* yylval ))

#define YY_INPUT(buf, result, max_size)                     \
{                                                           \
    input->read(buf, max_size);                             \
    result = input->gcount();                               \
}
    
%}

%option noyywrap
%option yylineno
/* %option never-interactive */

%%

#.*                                     /* ignore preprocessor directives */
[[:space:]]+                            /* eat spaces */
\/\*.*\*\/                              /* eat comment */
\/\/[^\n]*\n                            /* eat comment */        
class                                   { return T_CLASS; }
struct                                  { return T_STRUCT; }
static                                  { return T_STATIC; }
virtual                                 { return T_VIRTUAL; }
const                                   { return T_CONST; }
unsigned                                { return T_UNSIGNED; }
signed                                  { return T_SIGNED; }
void                                    { return T_VOID; }
bool                                    { return T_BOOL; }
char                                    { return T_CHAR; }
short                                   { return T_SHORT; }
int                                     { return T_INT; }
long                                    { return T_LONG; }
float                                   { return T_FLOAT; }
double                                  { return T_DOUBLE; }
public                                  { return T_PUBLIC; }
protected                               { return T_PROTECTED; }
private                                 { return T_PRIVATE; }
namespace                               { return T_NAMESPACE; }
[a-zA-Z_][a-zA-Z_0-9]*                  {
        Namespace* ns = search_namespace;
        if(ns == 0)
            ns = current_namespace;          
        // is it a type?
        for(std::vector<AtomicType*>::iterator i = ns->types.begin();
                i != ns->types.end(); ++i) {
            AtomicType* type = *i;
            if(type->name == yytext) {
                yylval->atomic_type = type;
                return T_ATOMIC_TYPE;
            }
        }
        // or a namespace? (hack for now...)
        yylval->_namespace = ns->_findNamespace(yytext, search_down);
        if(yylval->_namespace) {
            return T_NAMESPACEREF;
        }
        // a new ID
        yylval->str = strdup(yytext);
        return T_ID;
}
\:\:                                    { return T_DDCOL; }
[0-9]+                                  { 
                                            yylval->ival = atoi(yytext);
                                            return T_INT;
                                        }
[0-9]*\.[0-9]+(e[0-9]+)?                { 
                                            yylval->fval = atof(yytext);
                                            return T_FLOAT;
                                        }
\".*\"                                  {
                                            yylval->str = strdup(yytext);
                                            return T_STRING;
                                        }
.                                       { return yytext[0]; }

%%

