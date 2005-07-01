#ifndef __WRAPPERUTIL_HPP__
#define __WRAPPERUTIL_HPP__

#include <squirrel.h>
#include <exception>
#include <sstream>
#include <string>

struct WrappedFunction {
    const char* name;
    SQFUNCTION f;
};

template<typename T>
struct WrappedConstant {
    const char* name;
    T value;
};

struct WrappedClass {
    const char* name;
    WrappedFunction* functions;
    WrappedConstant<int>* int_consts;
    WrappedConstant<float>* float_consts;
    WrappedConstant<const char*>* string_consts;
};

class SquirrelError : public std::exception
{
public:
  SquirrelError(HSQUIRRELVM v, const std::string& message) throw();
  virtual ~SquirrelError() throw();

  const char* what() const throw();
private:
  std::string message;
};

void register_functions(HSQUIRRELVM v, WrappedFunction* functions);
void register_classes(HSQUIRRELVM v, WrappedClass* classes);

static inline void push_value(HSQUIRRELVM v, int val)
{
    sq_pushinteger(v, val);
}

static inline void push_value(HSQUIRRELVM v, float val)
{
    sq_pushfloat(v, val);
}

static inline void push_value(HSQUIRRELVM v, const char* str)
{
    sq_pushstring(v, str, -1);
}

template<typename T>
void _register_constants(HSQUIRRELVM v, WrappedConstant<T>* constants)
{
    for(WrappedConstant<T>* c = constants; c->name != 0; ++c) {
        sq_pushstring(v, c->name, -1);
        push_value(v, c->value);
        if(sq_createslot(v, -3) < 0) {
            std::stringstream msg;
            msg << "Couldn't register int constant '" << c->name << "'";
            throw SquirrelError(v, msg.str());
        }
    }
}

template<typename T>
void register_constants(HSQUIRRELVM v, WrappedConstant<T>* constants)
{
    sq_pushroottable(v);
    _register_constants(v, constants);
    sq_pop(v, 1);
}

void print_squirrel_stack(HSQUIRRELVM v);

#endif
