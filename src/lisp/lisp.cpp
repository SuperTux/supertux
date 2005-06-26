//  $Id$
//
//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de>
//  code in this file based on lispreader from Mark Probst
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#include <config.h>

#include "lisp.hpp"

namespace lisp
{
    
Lisp::Lisp(LispType newtype)
  : type(newtype)
{
}

Lisp::~Lisp()
{
  if(type == TYPE_SYMBOL || type == TYPE_STRING)
    delete[] v.string;
  if(type == TYPE_CONS) {
    delete v.cons.cdr;
    delete v.cons.car;
  }
}

Lisp*
Lisp::get_lisp(const char* name) const
{
  for(const Lisp* p = this; p != 0; p = p->get_cdr()) {
    Lisp* child = p->get_car();
    if(!child || child->get_type() != TYPE_CONS)
      continue;
    Lisp* childname = child->get_car();
    if(!childname)
      continue;
    std::string childName;
    if(!childname->get(childName))
      continue;
    if(childName == name) {
      return child->get_cdr();
    }
  }

  return 0;
}

void
Lisp::print(int indent) const
{
  for(int i = 0; i < indent; ++i)
    printf(" ");
  
  if(type == TYPE_CONS) {
    printf("(\n");
    const Lisp* lisp = this;
    while(lisp) {
      if(lisp->v.cons.car)
        lisp->v.cons.car->print(indent + 1);
      lisp = lisp->v.cons.cdr;
    }
    for(int i = 0; i < indent; ++i)
      printf(" ");
    printf(")");
  }
  if(type == TYPE_STRING) {
    printf("'%s' ", v.string);
  }
  if(type == TYPE_INTEGER) {
    printf("%d", v.integer);
  }
  if(type == TYPE_REAL) {
    printf("%f", v.real);
  }
  if(type == TYPE_SYMBOL) {
    printf("%s ", v.string);
  }
  if(type == TYPE_BOOLEAN) {
    printf("%s ", v.boolean ? "true" : "false");
  }
  printf("\n");
}

} // end of namespace lisp
