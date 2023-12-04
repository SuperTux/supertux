//  SuperTux
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
//                2023 Vankata453
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

#include "squirrel/autocomplete.hpp"

#include "squirrel/squirrel_virtual_machine.hpp"

namespace {

void sq_insert_commands(std::vector<std::string>& cmds, HSQUIRRELVM vm, const std::string& table_prefix, const std::string& search_prefix,
                        bool remove_prefix);

/**
 * Acts upon key, value on top of stack:
 * Appends key (plus type-dependent suffix) to cmds if table_prefix+key starts with search_prefix;
 * Calls sq_insert_commands if search_prefix starts with table_prefix+key (and value is a table/class/instance);
 */
void
sq_insert_command(std::vector<std::string>& cmds, HSQUIRRELVM vm, const std::string& table_prefix, const std::string& search_prefix,
                  bool remove_prefix)
{
  const SQChar* key_chars;
  if (SQ_FAILED(sq_getstring(vm, -2, &key_chars))) return;
  std::string key_string = table_prefix + key_chars;

  switch (sq_gettype(vm, -1))
  {
    case OT_INSTANCE:
      key_string += ".";
      if (search_prefix.substr(0, key_string.length()) == key_string)
      {
        sq_getclass(vm, -1);
        sq_insert_commands(cmds, vm, key_string, search_prefix, remove_prefix);
        sq_pop(vm, 1);
      }
      break;
    case OT_TABLE:
    case OT_CLASS:
      key_string += ".";
      if (search_prefix.substr(0, key_string.length()) == key_string)
        sq_insert_commands(cmds, vm, key_string, search_prefix, remove_prefix);
      break;
    case OT_CLOSURE:
    case OT_NATIVECLOSURE:
      key_string += "()";
      break;
    default:
      break;
  }

  if (key_string != search_prefix && key_string.substr(0, search_prefix.length()) == search_prefix)
  {
    if (remove_prefix)
    {
      const size_t pos = key_string.find_last_of('.', search_prefix.length());
      if (pos != std::string::npos)
      {
        std::string result = key_string.substr(pos);
        if (result != ".")
        {
          if (result.at(0) == '.')
            result.erase(0, 1);

          cmds.push_back(result);
        }
        return;
      }
    }
    cmds.push_back(key_string);
  }
}

/**
 * Calls sq_insert_command for all entries of table/class on top of stack.
 */
void
sq_insert_commands(std::vector<std::string>& cmds, HSQUIRRELVM vm, const std::string& table_prefix, const std::string& search_prefix,
                   bool remove_prefix)
{
  sq_pushnull(vm); // Push iterator.
  while (SQ_SUCCEEDED(sq_next(vm,-2)))
  {
    sq_insert_command(cmds, vm, table_prefix, search_prefix, remove_prefix);
    sq_pop(vm, 2); // Pop key, value.
  }
  sq_pop(vm, 1); // Pop iterator.
}

} // namespace

namespace squirrel {

std::vector<std::string>
autocomplete(const std::string& prefix, bool remove_prefix)
{
  std::vector<std::string> result;
  HSQUIRRELVM vm = SquirrelVirtualMachine::current()->get_vm().get_vm();

  // Append all keys of the current root table to list.
  sq_pushroottable(vm); // Push root table.
  while (true)
  {
    // Check all keys (and their children) for matches.
    sq_insert_commands(result, vm, "", prefix, remove_prefix);

    // Cycle through parent(delegate) table.
    SQInteger oldtop = sq_gettop(vm);
    if (SQ_FAILED(sq_getdelegate(vm, -1)) || oldtop == sq_gettop(vm))
      break;

    sq_remove(vm, -2); // Remove old table.
  }
  sq_pop(vm, 1); // Remove table.

  return result;
}

} // namespace squirrel
