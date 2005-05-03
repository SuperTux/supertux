%{

#include <iostream>
#include <sstream>
#include <stdexcept>
#include "tree.h"
#include "globals.h"

%}

%pure_parser
%union {
    char*       str;
    int         ival;
    float       fval;
    Class*      _class;
    Function*   function;
    Type*       type;
    AtomicType* atomic_type;
    Namespace*  _namespace;
}

%{

extern int yylex(YYSTYPE* yylval);
void yyerror(const char* s);
extern int yylineno;

static Class* currentClass = 0;
static Function* currentFunction = 0;
static Type* currentType = 0;
static ClassMember::Visbility currentVisibility;

class ParseError : public std::exception
{
public:
    ParseError(const std::string& message) throw()
    {
        std::ostringstream msg;
        msg << "Parse error in line " << yylineno << ": "
            << message;
        this->message = msg.str();
    }
    virtual ~ParseError() throw()
    {}
    const char* what() const throw()
    {
        return message.c_str();
    }

private:
    std::string message;
};

%}

%token <ival> T_INT
%token <fval> T_FLOAT
%token <str>  T_STRING
%token <str>  T_ID
%token <atomic_type> T_ATOMIC_TYPE
%token <_namespace> T_NAMESPACE;
%token T_CLASS
%token T_STRUCT
%token T_STATIC
%token T_CONST
%token T_UNSIGNED
%token T_SIGNED
%token T_VOID
%token T_BOOL
%token T_CHAR
%token T_SHORT
%token T_INT
%token T_LONG
%token T_FLOAT
%token T_DOUBLE
%token T_PUBLIC
%token T_PROTECTED
%token T_PRIVATE
%token T_DDCOL "::"

%type <_class> class_declaration
%type <function> function_declaration
%type <function> constructor_declaration;
%type <function> destructor_declaration;
%type <type>   type
%type <atomic_type> type_identifier

%%

input:  /* empty */
    | compilation_unit
;

compilation_unit: compilation_unit_part
    | compilation_unit compilation_unit_part
;

compilation_unit_part: class_declaration
        { unit->types.push_back($1); }  
    | function_declaration
        { unit->functions.push_back($1); }
;  

class_declaration: T_CLASS T_ID '{' 
            {
                currentClass = new Class();
                currentClass->name = $2;
                free($2);
                currentVisibility = ClassMember::PROTECTED;
            }
        class_body '}' ';'
            {
                $$ = currentClass;
            }
;

class_body: /* empty */
        | visibility_change class_body
        | constructor_declaration
            { 
                $1->visibility = currentVisibility;
                currentClass->members.push_back($1);
            }
          class_body
        | destructor_declaration
            {
                $1->visibility = currentVisibility;
                currentClass->members.push_back($1);
            }
          class_body
        | function_declaration
            {
                $1->visibility = currentVisibility;
                currentClass->members.push_back($1);
            }
          class_body
        | variable_declaration class_body
;

visibility_change:  T_PUBLIC ':'
            { currentVisibility = ClassMember::PUBLIC; }
        |   T_PROTECTED ':'
            { currentVisibility = ClassMember::PROTECTED; }
        |   T_PRIVATE ':'
            { currentVisibility = ClassMember::PRIVATE; }
;

constructor_declaration:    T_ID '('
        {
            currentFunction = new Function();
            currentFunction->type = Function::CONSTRUCTOR;
            free($1);
        }
    param_list ')' ';'
        {
            $$ = currentFunction;
        }
;

destructor_declaration:     '~' T_ID '(' ')' ';'
        {
            currentFunction = new Function();
            currentFunction->type = Function::DESTRUCTOR;
            free($2);
            $$ = currentFunction;
        }
;

variable_declaration:   type T_ID ';'

function_declaration:       type T_ID '(' 
        {
            currentFunction = new Function();
            currentFunction->type = Function::FUNCTION;
            currentFunction->return_type = *($1);
            delete $1;
            currentFunction->name = $2;
            free($2);
        }                           
    param_list ')' ';'
        {
            $$ = currentFunction;
        }
;

param_list: /* empty */
        | param_list2
;

param_list2: parameter
        | parameter ',' param_list2
;

parameter: type
            {
                Parameter parameter;
                parameter.type = *($1);
                delete $1;
                currentFunction->parameters.push_back(parameter);
            }
        | type T_ID
            {
                Parameter parameter;
                parameter.type = *($1);
                delete $1;
                parameter.name = *($2);
                free($2);
                currentFunction->parameters.push_back(parameter);
            }
;

type: {
          currentType = new Type();
      }
      prefix_type_modifiers atomic_type postfix_type_modifiers 
      {
          $$ = currentType;
      }
;

prefix_type_modifiers: /* empty */
            | T_UNSIGNED prefix_type_modifiers
            | T_SIGNED prefix_type_modifiers
            | T_STATIC prefix_type_modifiers
            | T_CONST prefix_type_modifiers
;

postfix_type_modifiers: /* empty */
            | T_CONST postfix_type_modifiers
                { currentType->_const = true; }
            |   '*' postfix_type_modifiers
                { currentType->pointer++; }
            |   '&' postfix_type_modifiers
                { currentType->ref++; }
;

atomic_type:    T_VOID      { currentType->atomic_type = &BasicType::VOID; }
            |   T_BOOL      { currentType->atomic_type = &BasicType::BOOL; }
            |   T_CHAR      { currentType->atomic_type = &BasicType::CHAR; }
            |   T_SHORT     { currentType->atomic_type = &BasicType::SHORT; }
            |   T_INT       { currentType->atomic_type = &BasicType::INT; }
            |   T_LONG      { currentType->atomic_type = &BasicType::LONG; }
            |   T_FLOAT     { currentType->atomic_type = &BasicType::FLOAT; }
            |   T_DOUBLE    { currentType->atomic_type = &BasicType::DOUBLE; }
            |   type_identifier  { currentType->atomic_type = $1; }
;

type_identifier: T_ATOMIC_TYPE
        {
            // search for type in current compilation unit...
            $$ = $1;
        }
        |   T_NAMESPACE "::" type_identifier
        {
            // hack...
            $$ = $3;
        }
;

%%

void yyerror(const char* error)
{
    throw ParseError(error);
}

