#ifndef __SERIALIZE_HPP__
#define __SERIALIZE_HPP__

#include <squirrel.h>
#include <string>
#include "lisp/lisp.hpp"
#include "lisp/writer.hpp"

namespace Scripting
{

  void save_squirrel_table(HSQUIRRELVM vm, int table_idx, lisp::Writer& writer);
  void load_squirrel_table(HSQUIRRELVM vm, int table_idx, const lisp::Lisp& lisp);

}

#endif


