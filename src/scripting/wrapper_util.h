#ifndef __WRAPPERUTIL_HPP__
#define __WRAPPERUTIL_HPP__

#include <squirrel.h>

struct WrappedFunction {
    const char* name;
    SQFUNCTION f;
};
struct WrappedClass {
    const char* name;
    WrappedFunction* functions;
};

void register_functions(HSQUIRRELVM v, WrappedFunction* functions);
void register_classes(HSQUIRRELVM v, WrappedClass* classes);

void expose_object(HSQUIRRELVM v, void* object, const char* type,
        const char* name);

#endif
