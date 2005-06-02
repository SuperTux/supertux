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

bool search_down = true;
Namespace* search_namespace = 0;
Namespace* current_namespace = 0;
static Class* current_class = 0;
static Function* currentFunction = 0;
static Type* current_type = 0;
static ClassMember::Visbility current_visibility;

class ParseError : public std::exception
{
public:
    ParseError(const std::string& message) throw()
    {
        std::ostringstream msg;
        msg << "Parse error in '" << current_file
            << "' line " << getCurrentLine() << ": "
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
%token <_namespace> T_NAMESPACEREF
%token T_CLASS
%token T_STRUCT
%token T_STATIC
%token T_VIRTUAL
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
%token T_NAMESPACE
%token T_DDCOL "::"

%type <_class> class_declaration
%type <function> function_declaration
%type <function> constructor_declaration;
%type <function> destructor_declaration;
%type <type>   type
%type <atomic_type> type_identifier

%%

input:
        {
            current_namespace = unit;
        }
    namespace_members
;

namespace_members: /* empty */
    | namespace_members namespace_member
;

namespace_declaration:
    T_NAMESPACE T_ID '{' 
        {
            Namespace* newNamespace = new Namespace();
            newNamespace->name = $2;
            free($2);
            current_namespace->add_namespace(newNamespace);
            current_namespace = newNamespace;
        }
    namespace_members '}'
        {
            current_namespace = current_namespace->parent;
        }
    | T_NAMESPACE T_NAMESPACEREF '{'
        {
            current_namespace = $2;
        }
    namespace_members '}'
        {
            current_namespace = current_namespace->parent;
        }
;

namespace_member:
    class_declaration
        { current_namespace->add_type($1); }
    | function_declaration
        { current_namespace->functions.push_back($1); }
    | namespace_declaration
;  

class_declaration:
    T_CLASS T_ID '{' 
        {
            current_class = new Class();
            current_class->name = $2;
            free($2);
            current_class->docu_comment = last_docucomment;
            last_docucomment = "";
            current_visibility = ClassMember::PROTECTED;
        }
    class_body '}' ';'
        {
            $$ = current_class;
        }
;

class_body: /* empty */
        | class_body class_body_element
;

class_body_element:
        visibility_change
        | constructor_declaration
            { 
                $1->visibility = current_visibility;
                current_class->members.push_back($1);
            }
        | destructor_declaration
            {
                $1->visibility = current_visibility;
                current_class->members.push_back($1);
            }
        | function_declaration
            {
                $1->visibility = current_visibility;
                current_class->members.push_back($1);
            }
        | variable_declaration
;

visibility_change:
    T_PUBLIC ':'
        { current_visibility = ClassMember::PUBLIC; }
    |   T_PROTECTED ':'
        { current_visibility = ClassMember::PROTECTED; }
    |   T_PRIVATE ':'
        { current_visibility = ClassMember::PRIVATE; }
;

constructor_declaration:    
    T_ID '('
        {
            currentFunction = new Function();
            currentFunction->type = Function::CONSTRUCTOR;
            currentFunction->docu_comment = last_docucomment;
            last_docucomment = "";
            free($1);
        }
    parameter_list ')' ';'
        {
            $$ = currentFunction;
        }
;

destructor_declaration:
    maybe_virtual '~' T_ID '(' ')' abstract_declaration ';'
        {
            currentFunction = new Function();
            currentFunction->type = Function::DESTRUCTOR;
            currentFunction->docu_comment = last_docucomment;
            last_docucomment = "";
            free($3);
            $$ = currentFunction;
        }
;

maybe_virtual:
    /* empty */
    | T_VIRTUAL
;

variable_declaration:
    type T_ID ';'
;

function_declaration:
    maybe_virtual type T_ID '(' 
        {
            currentFunction = new Function();
            currentFunction->type = Function::FUNCTION;
            currentFunction->return_type = *($2);
            delete $2;
            currentFunction->name = $3;
            free($3);
            currentFunction->docu_comment = last_docucomment;
            last_docucomment = "";
        }                           
    parameter_list ')' abstract_declaration ';'
        {
            $$ = currentFunction;
        }
;

abstract_declaration:
    /* empty */
    | '=' T_INT
;

parameter_list:
    /* empty */
    | parameters
;

parameters:
    parameter
    | parameters ',' parameter
;

parameter:
    type
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

type:
        {
            current_type = new Type();
        }
    prefix_type_modifiers atomic_type postfix_type_modifiers 
        {
            $$ = current_type;
        }
;

prefix_type_modifiers:
    /* empty */
    | prefix_type_modifiers prefix_type_modifier
;

prefix_type_modifier:
    T_UNSIGNED
        { current_type->_unsigned = true; }
    | T_SIGNED
        { current_type->_unsigned = false; }
    | T_STATIC
        { current_type->_static = true; }
    | T_CONST
        { current_type->_const = true; }
;

postfix_type_modifiers:
    /* empty */
    | postfix_type_modifiers postfix_type_modifier
;

postfix_type_modifier:
    T_CONST
        { current_type->_const = true; }
    |   '*'
        { current_type->pointer++; }
    |   '&'
        { current_type->ref++; }
;

atomic_type:
    T_VOID
        { current_type->atomic_type = &BasicType::VOID; }
    | T_BOOL
        { current_type->atomic_type = &BasicType::BOOL; }
    | T_CHAR
        { current_type->atomic_type = &BasicType::CHAR; }
    | T_SHORT
        { current_type->atomic_type = &BasicType::SHORT; }
    | T_INT
        { current_type->atomic_type = &BasicType::INT; }
    | T_LONG
        { current_type->atomic_type = &BasicType::LONG; }
    | T_FLOAT
        { current_type->atomic_type = &BasicType::FLOAT; }
    | T_DOUBLE
        { current_type->atomic_type = &BasicType::DOUBLE; }
    | type_identifier
        { current_type->atomic_type = $1; }
;

type_identifier:
    T_ATOMIC_TYPE
        {
            // search for type in current compilation unit...
            $$ = $1;
        }
    | namespace_refs "::" T_ATOMIC_TYPE
        {
            $$ = $3;
            search_namespace = 0;
            search_down = true;
        }
;

namespace_refs:
    T_NAMESPACEREF
        {
            search_namespace = $1;
            search_down = false;
        }
    | namespace_refs "::" T_NAMESPACEREF
        {
            search_namespace = $3;
        }
;

%%

void yyerror(const char* error)
{
    throw ParseError(error);
}

