#include <config.h>

#include "list_iterator.h"
#include <stdexcept>

namespace lisp
{

ListIterator::ListIterator(const lisp::Lisp* newlisp)
  : current_lisp(0), cur(newlisp)
{
}

bool
ListIterator::next()
{
  if(cur == 0)
    return false;

  const lisp::Lisp* child = cur->get_car();
  if(!child)
    throw new std::runtime_error("child is 0 in list entry");
  if(child->get_type() != lisp::Lisp::TYPE_CONS)
    throw new std::runtime_error("Expected CONS");
  const lisp::Lisp* name = child->get_car();
  if(!name || name->get_type() != lisp::Lisp::TYPE_SYMBOL)
    throw new std::runtime_error("Expected symbol");
  name->get(current_item);
  current_lisp = child->get_cdr();

  cur = cur->get_cdr();
  return true;
}

}
