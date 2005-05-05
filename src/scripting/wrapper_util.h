#ifndef __WRAPPERUTIL_HPP__
#define __WRAPPERUTIL_HPP__

#include <squirrel.h>
#include <exception>
#include <string>

struct WrappedFunction {
    const char* name;
    SQFUNCTION f;
};
struct WrappedClass {
    const char* name;
    WrappedFunction* functions;
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

void print_squirrel_stack(HSQUIRRELVM v);

#endif
