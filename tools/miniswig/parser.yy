%{
//#include <config.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include "tree.hpp"
#include "globals.hpp"

%}

%pure-parser
%union {
    char*       str;
    int         ival;
    float       fval;
    Class*      _class;
    Function*   function;
    Field*      field;
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
static Function* current_function = 0;
static Type* current_type = 0;
static Field* current_field = 0;
static ClassMember::Visibility current_visibility;

class ParseError : public std::exception
{
public:
    ParseError(const std::string& message) throw() :
         message()
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
%token T_SUSPEND
%token T_CUSTOM
%token T_CONST
%token T_UNSIGNED
%token T_SIGNED
%token T_VOID
%token T_BOOL
%token T_CHAR
%token T_SHORT
%token T_LONG
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
%type <field> field_declaration;
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
    | field_declaration
        { current_namespace->fields.push_back($1); }
;  

class_declaration:
    T_CLASS T_ID 
        {
            current_class = new Class();
            current_class->name = $2;
            free($2);
            current_class->docu_comment = last_docucomment;
            last_docucomment = "";
            current_visibility = ClassMember::PROTECTED;
        }
    superclass_list '{' class_body '}' ';'
        {
            $$ = current_class;
        }
;

superclass_list:
    /* empty */
    | ':' superclasses
;

superclasses:
      superclass
    | superclasses ',' superclass
;

superclass:
    superclass_visibility type_identifier
        {
            Class* superclass = dynamic_cast<Class*> ($2);
            if(superclass == 0)
                throw ParseError("SuperClass is not a Class type");
            current_class->super_classes.push_back(superclass);
            superclass->sub_classes.push_back(current_class);
        }
;

superclass_visibility:
    T_PUBLIC
    | T_PROTECTED
    | T_PRIVATE
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
        | field_declaration
            {
                $1->visibility = current_visibility;
                current_class->members.push_back($1);
            }
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
            current_function = new Function();
            current_function->type = Function::CONSTRUCTOR;
            current_function->docu_comment = last_docucomment;
            last_docucomment = "";
            free($1);
        }
    parameter_list ')' ';'
        {
            $$ = current_function;
        }
;

destructor_declaration:
    '~' T_ID '(' ')' abstract_declaration ';'
        {
            current_function = new Function();
            current_function->type = Function::DESTRUCTOR;
            current_function->docu_comment = last_docucomment;
            last_docucomment = "";
            free($2);
            $$ = current_function;
        }
;

field_declaration:
    type T_ID 
        {
            current_field = new Field();
            current_field->type = $1;
            current_field->docu_comment = last_docucomment;
            last_docucomment = "";
            current_field->name = $2;
            free($2);
        }
    maybe_const_initialisation ';'
        {
            $$ = current_field;
        }
;

maybe_const_initialisation:
    /* empty */
    | '=' T_INT
        {
            if(current_field->type->atomic_type == &BasicType::FLOAT) {
                current_field->const_float_value = (float) $2;
            } else {
                current_field->const_int_value = $2;
            }
            current_field->has_const_value = true;
        }
    | '=' T_FLOAT
        {
            current_field->const_float_value = $2;
            current_field->has_const_value = true;
        }
    | '=' T_STRING
        {
            current_field->const_string_value = $2;
            current_field->has_const_value = true;
        }
;          

function_declaration:
    type T_ID '(' 
        {
            current_function = new Function();
            current_function->type = Function::FUNCTION;
            current_function->return_type = *($1);
            delete $1;
            current_function->name = $2;
            free($2);
            current_function->docu_comment = last_docucomment;
            last_docucomment = "";
        }                           
    parameter_list ')' function_attributes abstract_declaration ';'
        {
            $$ = current_function;
        }
;

function_attributes:
    /* empty */
    | T_CONST function_attributes
    | T_CUSTOM '(' T_STRING ')' function_attributes
      {
        current_function->parameter_spec = $3;
        current_function->custom = true;
      }
    | T_SUSPEND function_attributes
      {
        current_function->suspend = true;
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
            current_function->parameters.push_back(parameter);
        }
    | type T_ID
        {
            Parameter parameter;
            parameter.type = *($1);
            delete $1;
            parameter.name = $2;
            free($2);
            current_function->parameters.push_back(parameter);
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

__attribute__((noreturn))
void yyerror(const char* error)
{
    throw ParseError(error);
}

