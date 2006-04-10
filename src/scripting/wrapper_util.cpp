//  $Id: main.cpp 3275 2006-04-09 00:32:34Z sommer $
// 
//  SuperTux
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
#include <config.h>

#include <stdexcept>
#include <sstream>
#include "wrapper_util.hpp"

namespace Scripting
{

std::string squirrel2string(HSQUIRRELVM v, int i)
{
  std::ostringstream os;
  switch(sq_gettype(v, i))
    {
    case OT_NULL:
      os << "<null>";        
      break;
    case OT_BOOL: {
      SQBool p;
      sq_getbool(v, i, &p);
      if (p) 
        os << "true";
      else
        os << "false";
      break;
    }
    case OT_INTEGER: {
      int val;
      sq_getinteger(v, i, &val);
      os << val;
      break;
    }
    case OT_FLOAT: {
      float val;
      sq_getfloat(v, i, &val);
      os << val;
      break;
    }
    case OT_STRING: {
      const char* val;
      sq_getstring(v, i, &val);
      os << "\"" << val << "\"";
      break;    
    }
    case OT_TABLE: {
      bool first = true;
      os << "{";
      sq_pushnull(v);  //null iterator
      while(SQ_SUCCEEDED(sq_next(v,i-1)))
        {
          if (!first) {
            os << ", ";
          }
          first = false;

          //here -1 is the value and -2 is the key
          os << squirrel2string(v, -2) << " => " 
             << squirrel2string(v, -1);
                              
          sq_pop(v,2); //pops key and val before the nex iteration
        }
      sq_pop(v, 1);
      os << "}";
      break;
    }
    case OT_ARRAY: {
      bool first = true;
      os << "[";
      sq_pushnull(v);  //null iterator
      while(SQ_SUCCEEDED(sq_next(v,i-1)))
        {
          if (!first) {
            os << ", ";
          }
          first = false;

          //here -1 is the value and -2 is the key
          // we ignore the key, since that is just the index in an array
          os << squirrel2string(v, -1);
                              
          sq_pop(v,2); //pops key and val before the nex iteration
        }
      sq_pop(v, 1);
      os << "]";
      break;
    }
    case OT_USERDATA:
      os << "<userdata>";
      break;
    case OT_CLOSURE:        
      os << "<closure (function)>";
      break;
    case OT_NATIVECLOSURE:
      os << "<native closure (C function)>";
      break;
    case OT_GENERATOR:
      os << "<generator>";
      break;
    case OT_USERPOINTER:
      os << "userpointer";
      break;
    case OT_THREAD:
      os << "<thread>";
      break;
    case OT_CLASS:
      os << "<class>";
      break;
    case OT_INSTANCE:
      os << "<instance>";
      break;
    default:
      os << "<unknown>";
      break;
    }
  return os.str();
}

void print_squirrel_stack(HSQUIRRELVM v)
{
    printf("--------------------------------------------------------------\n");
    int count = sq_gettop(v);
    for(int i = 1; i <= count; ++i) {
        printf("%d: ",i);
        switch(sq_gettype(v, i))
        {
            case OT_NULL:
                printf("null");        
                break;
            case OT_INTEGER: {
                int val;
                sq_getinteger(v, i, &val);
                printf("integer (%d)", val);
                break;
            }
            case OT_FLOAT: {
                float val;
                sq_getfloat(v, i, &val);
                printf("float (%f)", val);
                break;
            }
            case OT_STRING: {
                const char* val;
                sq_getstring(v, i, &val);
                printf("string (%s)", val);
                break;    
            }
            case OT_TABLE:
                printf("table");
                break;
            case OT_ARRAY:
                printf("array");
                break;
            case OT_USERDATA:
                printf("userdata");
                break;
            case OT_CLOSURE:        
                printf("closure(function)");    
                break;
            case OT_NATIVECLOSURE:
                printf("native closure(C function)");
                break;
            case OT_GENERATOR:
                printf("generator");
                break;
            case OT_USERPOINTER:
                printf("userpointer");
                break;
            case OT_THREAD:
                printf("thread");
                break;
            case OT_CLASS:
                printf("class");
                break;
            case OT_INSTANCE:
                printf("instance");
                break;
            default:
                printf("unknown?!?");
                break;
        }
        printf("\n");
    }
    printf("--------------------------------------------------------------\n");
}

static SQInteger squirrel_read_char(SQUserPointer file)
{
  std::istream* in = reinterpret_cast<std::istream*> (file);
  char c = in->get();
  if(in->eof())
    return 0;
  return c;
}

void compile_script(HSQUIRRELVM vm, std::istream& in, const std::string& sourcename)
{
  if(SQ_FAILED(sq_compile(vm, squirrel_read_char, &in, sourcename.c_str(), true)))
    throw SquirrelError(vm, "Couldn't parse script");  
}

void compile_and_run(HSQUIRRELVM vm, std::istream& in, const std::string& sourcename)
{
  compile_script(vm, in, sourcename);
  
  int oldtop = sq_gettop(vm);

  try {
    sq_pushroottable(vm);
    if(SQ_FAILED(sq_call(vm, 1, false)))
      throw SquirrelError(vm, "Couldn't start script");
  } catch(...) {
    sq_settop(vm, oldtop);
    throw;
  }

  sq_settop(vm, oldtop);
}

}
