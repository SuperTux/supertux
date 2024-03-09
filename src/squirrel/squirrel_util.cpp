//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "squirrel/squirrel_util.hpp"

#include <simplesquirrel/array.hpp>
#include <simplesquirrel/table.hpp>

#include "util/log.hpp"

std::string squirrel_to_string(const ssq::Object& object)
{
  std::ostringstream os;
  switch (object.getType())
  {
    case ssq::Type::NULLPTR:
      os << "<null>";
      break;
    case ssq::Type::BOOL:
      os << object.toBool();
      break;
    case ssq::Type::INTEGER:
      os << object.to<int>();
      break;
    case ssq::Type::FLOAT:
      os << object.toFloat();
      break;
    case ssq::Type::STRING:
      os << "\"" << object.toString() << "\"";
      break;
    case ssq::Type::TABLE:
    {
      const std::map<std::string, ssq::Object> table = object.toTable().convertRaw();

      bool first = true;
      os << "{";
      for (const auto& [key, value] : table)
      {
        if (!first)
          os << ", ";
        first = false;

        os << key << " => " << squirrel_to_string(value);
      }
      os << "}";
      break;
    }
    case ssq::Type::ARRAY:
    {
      const std::vector<ssq::Object> array = object.toArray().convertRaw();

      bool first = true;
      os << "[";
      for (const ssq::Object& value : array)
      {
        if (!first)
          os << ", ";
        first = false;

        os << squirrel_to_string(value);
      }
      os << "]";
      break;
    }
    case ssq::Type::USERDATA:
      os << "<userdata>";
      break;
    case ssq::Type::CLOSURE:
      os << "<closure>";
      break;
    case ssq::Type::NATIVECLOSURE:
      os << "<native closure>";
      break;
    case ssq::Type::GENERATOR:
      os << "<generator>";
      break;
    case ssq::Type::USERPOINTER:
      os << "userpointer";
      break;
    case ssq::Type::THREAD:
      os << "<thread>";
      break;
    case ssq::Type::CLASS:
      os << "<class>";
      break;
    case ssq::Type::INSTANCE:
      os << "<instance>";
      break;
    case ssq::Type::WEAKREF:
      os << "<weakref>";
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
  SQInteger count = sq_gettop(v);
  for (int i = 1; i <= count; i++)
    {
    printf("%d: ", i);
    switch (sq_gettype(v, i))
    {
      case OT_NULL:
        printf("null");
        break;
      case OT_INTEGER:
      {
        SQInteger val;
        sq_getinteger(v, i, &val);
        printf("integer (%d)", static_cast<int>(val));
        break;
      }
      case OT_FLOAT:
      {
        SQFloat val;
        sq_getfloat(v, i, &val);
        printf("float (%f)", static_cast<double>(val));
        break;
      }
      case OT_STRING:
      {
        const SQChar* val;
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
      case OT_WEAKREF:
        printf("weakref");
        break;
      default:
        printf("unknown?!?");
        break;
    }
    printf("\n");
  }
  printf("--------------------------------------------------------------\n");
}

/* EOF */
