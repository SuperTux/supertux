#ifndef __LISP_ITERATOR_H__
#define __LISP_ITERATOR_H__

#include "lisp/lisp.h"

namespace lisp
{

/**
 * Small and a bit hacky helper class that helps parsing lisp lists where all
 * entries are lists again themselves
 */
class ListIterator
{
public:
  ListIterator(const lisp::Lisp* cur);
  
  const std::string& item() const
  { return current_item; }
  lisp::Lisp* lisp() const
  { return current_lisp; }
  lisp::Lisp* value() const
  { return current_lisp->get_car(); }
  bool next();

private:
  std::string current_item;
  lisp::Lisp* current_lisp;
  const lisp::Lisp* cur;
};

}

#endif

