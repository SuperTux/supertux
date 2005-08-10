#ifndef __WRAPPERUTIL_HPP__
#define __WRAPPERUTIL_HPP__

#include <squirrel.h>
#include <sstream>
#include <string>

std::string squirrel2string(HSQUIRRELVM v, int i);
void print_squirrel_stack(HSQUIRRELVM v);

#endif
