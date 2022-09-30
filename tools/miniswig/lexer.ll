%{
//#include <config.h>
  
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "tree.hpp"
#include "parser.hpp"
#include "globals.hpp"

// there seems to be a bug in flex that adds some ECHO directives
// in some rules, we don't need debug output
#define ECHO  {}

#define YY_NEVER_INTERACTIVE 1
#define YY_DECL int yylex(YYSTYPE* yylval)

#define YY_INPUT(buf, result, max_size)                     \
{                                                           \
    input->read(buf, max_size);                             \
    result = input->gcount();                               \
}

std::string last_docucomment;
std::string original_file;
std::string current_file;
std::string comm;
int offset_lnum;

int getCurrentLine()
{
    return yylineno - offset_lnum;
}
    
%}

%option noyywrap
%option yylineno
/* %option never-interactive */

%x comment
%%

#[ \t]+[0-9]+[ \t]+.*                         {
    int lnum;
    char file[1024];
    if(sscanf(yytext, "# %d \"%1023[^\"]\"", &lnum, file) == 2) {
        offset_lnum = yylineno - lnum + 1;
        current_file = file;
        if(original_file == "")
            original_file = file;
    } else {
        std::cerr << "Warning: Parse error in processor info directive.\n";
    }
}
#.*                                     /* ignore preprocessor directives */
[[:space:]]+                            /* eat spaces */
"/*"                                    { BEGIN(comment); comm = ""; }
<comment>[^*\n]*                        { comm += yytext; }
<comment>"*"+[^*/]*                     { comm += yytext; }
<comment>"*/"                    {
    BEGIN(INITIAL);
    if(comm[0] == '*') { // It's a docu comment...
        last_docucomment = "";
        bool linestart = true;
        for(size_t i = 1; i < comm.size(); ++i) {
            if(linestart && (comm[i] == '*' || isspace(comm[i]))) {
                continue;
            } else if(comm[i] == '\n') {
                linestart = true;
            } else {
                linestart = false;
            }
            last_docucomment += comm[i];
        }
    }
}
\/\/[^\n]*\n                            {
    if(yytext[2] == '/') { // it's a docu comment...
        last_docucomment = std::string(yytext+3, strlen(yytext)-4);
    }
}
class                                   { return T_CLASS; }
struct                                  { return T_STRUCT; }
static                                  { return T_STATIC; }
virtual                                 { }
final                                   { }
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
__suspend                               { return T_SUSPEND; }
__custom                                { return T_CUSTOM; }
[a-zA-Z_][a-zA-Z_0-9]*                  {
        Namespace* ns = search_namespace;
        if(ns == 0)
            ns = current_namespace;          
        // is it a type?
        yylval->atomic_type = ns->_findType(yytext, search_down);
        if(yylval->atomic_type) {
            return T_ATOMIC_TYPE;
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
(0x)?[0-9]+ {
        sscanf(yytext, "%i", &(yylval->ival));
        return T_INT;
}
[0-9]*\.[0-9]+(e[0-9]+)? { 
        sscanf(yytext, "%f", &(yylval->fval));
        return T_FLOAT;
}
\".*\" {
        yylval->str = strdup(yytext);
        return T_STRING;
}
.                                       { return yytext[0]; }

%%

