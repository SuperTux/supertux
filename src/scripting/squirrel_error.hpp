#ifndef __SQUIRREL_ERROR_HPP__
#define __SQUIRREL_ERROR_HPP__

#include <squirrel.h>
#include <stdexcept>

namespace Scripting
{

/** Exception class for squirrel errors, it takes a squirrelvm and uses
 * sq_geterror() to retrieve additional information about the last error that
 * occured and creates a readable message from that.
 */
class SquirrelError : public std::exception
{
public:
  SquirrelError(HSQUIRRELVM v, const std::string& message) throw();
  virtual ~SquirrelError() throw();

  const char* what() const throw();
private:
  std::string message;
};

}

#endif

