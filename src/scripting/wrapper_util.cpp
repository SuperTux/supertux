#include <config.h>

#include <stdexcept>
#include <sstream>
#include "wrapper_util.hpp"

static void register_function(HSQUIRRELVM v, SQFUNCTION func, const char* name)
{
    sq_pushstring(v, name, -1);
    sq_newclosure(v, func, 0); //create a new function
    sq_createslot(v, -3);
}

static void register_class(HSQUIRRELVM v, WrappedClass* wclass)
{
    sq_pushstring(v, wclass->name, -1);
    sq_newclass(v, false);
    for(WrappedFunction* func = wclass->functions; func->name != 0; ++func) {
        register_function(v, func->f, func->name);
    }
    sq_createslot(v, -3);
}

void register_functions(HSQUIRRELVM v, WrappedFunction* functions)
{
    sq_pushroottable(v);
    for(WrappedFunction* func = functions; func->name != 0; ++func) {
        register_function(v, func->f, func->name);
    }
    sq_pop(v, 1);
}

void register_classes(HSQUIRRELVM v, WrappedClass* classes)
{
    sq_pushroottable(v);
    for(WrappedClass* wclass = classes; wclass->name != 0; ++wclass) {
        register_class(v, wclass);
    }
    sq_pop(v, 1);
}

void print_squirrel_stack(HSQUIRRELVM v)
{
    printf("--------------------------------------------------------------\n");
    int count = sq_gettop(v);
    for(int i = 1; i <= count; ++i) {
        printf("%d: ",i);
        switch(sq_gettype(v, i))
        {
            case OT_NULL:
                printf("null");        
                break;
            case OT_INTEGER: {
                int val;
                sq_getinteger(v, i, &val);
                printf("integer (%d)", val);
                break;
            }
            case OT_FLOAT: {
                float val;
                sq_getfloat(v, i, &val);
                printf("float (%f)", val);
                break;
            }
            case OT_STRING: {
                const char* val;
                sq_getstring(v, i, &val);
                printf("string (%s)", val);
                break;    
            }
            case OT_TABLE:
                printf("table");
                break;
            case OT_ARRAY:
                printf("array");
                break;
            case OT_USERDATA:
                printf("userdata");
                break;
            case OT_CLOSURE:        
                printf("closure(function)");    
                break;
            case OT_NATIVECLOSURE:
                printf("native closure(C function)");
                break;
            case OT_GENERATOR:
                printf("generator");
                break;
            case OT_USERPOINTER:
                printf("userpointer");
                break;
            case OT_CLASS:
                printf("class");
                break;
            case OT_INSTANCE:
                printf("instance");
                break;
            default:
                printf("unknown?!?");
                break;
        }
        printf("\n");
    }
    printf("--------------------------------------------------------------\n");
}

//----------------------------------------------------------------------------

SquirrelError::SquirrelError(HSQUIRRELVM v, const std::string& message) throw()
{
  std::ostringstream msg;
  msg << "SQuirrel error: " << message << " (";
  const char* lasterr;
  sq_getlasterror(v);
  sq_getstring(v, -1, &lasterr);
  sq_pop(v, 1);
  msg << lasterr << ")";
  this->message = msg.str();
}

SquirrelError::~SquirrelError() throw()
{}

const char*
SquirrelError::what() const throw()
{
  return message.c_str();
}
