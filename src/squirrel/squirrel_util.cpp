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
  switch (object.getType())
  {
    case ssq::Type::NULLPTR:
      os << "<null>";
      break;
    case ssq::Type::BOOL:
    {
      auto bool_value = object.toBool();
      if (lisp_syntax)
        os << (bool_value ? "#t" : "#f");
      else
        os << bool_value;
    }
      break;
    case ssq::Type::INTEGER:
      os << object.to<int>();
      break;
    case ssq::Type::FLOAT:
      os << object.toFloat();
      break;
    case ssq::Type::STRING:
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
    case ssq::Type::TABLE:
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
    case ssq::Type::ARRAY:
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
