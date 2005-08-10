#include <config.h>

#include "squirrel_error.hpp"
#include <sstream>

namespace Scripting
{

SquirrelError::SquirrelError(HSQUIRRELVM v, const std::string& message) throw()
{
  std::ostringstream msg;
  msg << "Squirrel error: " << message << " (";
  const char* lasterr;
  sq_getlasterror(v);
  if(sq_gettype(v, -1) != OT_STRING)
  {
    lasterr = "no error info";
  }
  else
  {
    sq_getstring(v, -1, &lasterr);
  }
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

}
