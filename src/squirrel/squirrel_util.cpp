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

std::string squirrel_to_string(const ssq::Object& object, bool lisp_syntax)
{
  std::ostringstream os;
  using Type = ssq::Type;
  const auto& obj_type = object.getType();

  // Data types that can not directly be written in lisp should return a null value
  if (lisp_syntax &&
      (obj_type != Type::BOOL && obj_type != Type::INTEGER && obj_type != Type::FLOAT &&
       obj_type != Type::STRING && obj_type != Type::TABLE && obj_type != Type::ARRAY))
  {
    os << "nil";
    return os.str();
  }

  switch (obj_type)
  {
    case Type::NULLPTR:
      os << "<null>";
      break;
    case Type::BOOL:
    {
      auto bool_value = object.toBool();
      if (lisp_syntax)
        os << (bool_value ? "#t" : "#f");
      else
        os << bool_value;
    }
      break;
    case Type::INTEGER:
      os << object.to<int>();
      break;
    case Type::FLOAT:
      os << object.toFloat();
      break;
    case Type::STRING:
    {
      auto string_value = object.toString();
      if (lisp_syntax)
      {
        os << string_value;
      }
      else
      {
        os << "\"" << string_value << "\"";
      }
    }
      break;
    case Type::TABLE:
    {
      const std::map<std::string, ssq::Object> table = object.toTable().convertRaw();

      bool first = true;
      if (!lisp_syntax)
        os << "{";

      for (const auto& [key, value] : table)
      {
        if (!first)
        {
          if (lisp_syntax)
          {
            os << " ";
          }
          else
          {
            os << ", ";
          }
        }
        first = false;

        if (lisp_syntax)
        {
          os << "(" << key << " " << squirrel_to_string(value, lisp_syntax) << ")";
        }
        else
        {
          os << key << " => " << squirrel_to_string(value);
        }
      }

      if (!lisp_syntax)
        os << "}";
      break;
    }
    case Type::ARRAY:
    {
      const std::vector<ssq::Object> array = object.toArray().convertRaw();

      bool first = true;
      if (!lisp_syntax)
        os << "[";
      
      for (const ssq::Object& value : array)
      {
        if (!first)
        {
          if (lisp_syntax)
          {
            os << " ";
          }
          else
          {
            os << ", ";
          }
        }
        first = false;

        os << squirrel_to_string(value, lisp_syntax);
      }
      if (!lisp_syntax)
        os << "]";
      break;
    }

    // Following cases are only for non-LISP syntax (Lisp syntax got handled earlier)
    case Type::USERDATA:
      os << "<userdata>";
      break;
    case Type::CLOSURE:
      os << "<closure>";
      break;
    case Type::NATIVECLOSURE:
      os << "<native closure>";
      break;
    case Type::GENERATOR:
      os << "<generator>";
      break;
    case Type::USERPOINTER:
      os << "<userpointer>";
      break;
    case Type::THREAD:
      os << "<thread>";
      break;
    case Type::CLASS:
      os << "<class>";
      break;
    case Type::INSTANCE:
      os << "<instance>";
      break;
    case Type::WEAKREF:
      os << "<weakref>";
      break;
    default:
      os << "<unknown>";
      break;
  }
  return os.str();
}
