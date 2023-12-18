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

#include "squirrel/suggestions.hpp"

#include <algorithm>

#include "squirrel/squirrel_virtual_machine.hpp"

static const std::string SCRIPTING_REFERENCE_FILE = "scripts/reference.stsr";

namespace squirrel {

Suggestion::Suggestion(const std::string& name_, const ScriptingObject* ref_,
                       const bool is_instance_) :
  name(name_),
  reference(ref_),
  is_instance(is_instance_)
{
}


void sq_insert_commands(SuggestionStack& cmds, HSQUIRRELVM vm,
                        const std::string& table_prefix, std::vector<const ScriptingClass*> parent_classes,
                        const std::string& search_prefix, bool remove_prefix);

/**
 * Acts upon key, value on top of stack:
 * Appends key (plus type-dependent suffix) to cmds if table_prefix+key starts with search_prefix;
 * Calls sq_insert_commands if search_prefix starts with table_prefix+key (and value is a table/class/instance);
 */
void
sq_insert_command(SuggestionStack& cmds, HSQUIRRELVM vm,
                  const std::string& table_prefix, const std::vector<const ScriptingClass*>& parent_classes,
                  const std::string& search_prefix, bool remove_prefix)
{
  const SQChar* key_chars;
  if (SQ_FAILED(sq_getstring(vm, -2, &key_chars))) return;
  std::string base_name = key_chars;
  std::string key_string = table_prefix + base_name;

  const ScriptingObject* object = nullptr;

  const SQObjectType type = sq_gettype(vm, -1);
  switch (type)
  {
    case OT_INSTANCE:
    {
      key_string += ".";

      // Search for class name
      bool found_class = false;
      sq_pushroottable(vm); // Push root table.
      sq_pushnull(vm); // Push iterator.
      while (!found_class && SQ_SUCCEEDED(sq_next(vm, -2)))
      {
        if (sq_gettype(vm, -1) == OT_CLASS)
        {
          sq_push(vm, -5); // Push instance.
          if (sq_instanceof(vm))
          {
            if (SQ_FAILED(sq_getstring(vm, -3, &key_chars))) break;
            found_class = true;
          }
          sq_pop(vm, 1); // Pop instance.
        }
        sq_pop(vm, 2); // Pop key, value.
      }
      sq_pop(vm, 2); // Pop iterator, root table.

      std::vector<const ScriptingClass*> classes;
      if (found_class) // Succeeded getting class name
      {
        base_name = key_chars; // Use class name

        for (const auto& root : s_scripting_roots)
        {
          auto it = std::find_if(root->classes.begin(), root->classes.end(),
                                 [&base_name](const auto& cl) { return cl.name == base_name; });
          if (it != root->classes.end())
          {
            classes.push_back(&*it);
            if (!object)
              object = classes.back();
          }
        }
      }

      if (search_prefix.substr(0, key_string.length()) == key_string)
      {
        sq_getclass(vm, -1); // Push class.
        sq_insert_commands(cmds, vm, key_string, classes, search_prefix, remove_prefix);
        sq_pop(vm, 1); // Pop class.
      }
    }
    break;

    case OT_TABLE:
    {
      key_string += ".";

      if (search_prefix.substr(0, key_string.length()) == key_string)
        sq_insert_commands(cmds, vm, key_string, {}, search_prefix, remove_prefix);
    }
    break;

    case OT_CLASS:
    {
      key_string += ".";

      std::vector<const ScriptingClass*> classes;
      for (const auto& root : s_scripting_roots)
      {
        auto it = std::find_if(root->classes.begin(), root->classes.end(),
                               [&base_name](const auto& cl) { return cl.name == base_name; });
        if (it != root->classes.end())
        {
          classes.push_back(&*it);
          if (!object)
            object = classes.back();
        }
      }

      if (search_prefix.substr(0, key_string.length()) == key_string)
        sq_insert_commands(cmds, vm, key_string, classes, search_prefix, remove_prefix);
    }
    break;

    case OT_CLOSURE:
    {
      key_string += "()";
    }
    break;

    case OT_NATIVECLOSURE:
    {
      key_string += "()";
      for (const ScriptingClass* cl : parent_classes)
      {
        if (!cl) continue;

        auto it = std::find_if(cl->functions.begin(), cl->functions.end(),
                               [&base_name](const auto& func) { return func.name == base_name; });
        if (it != cl->functions.end())
        {
          object = &*it;
          break;
        }
      }
    }
    break;

    case OT_INTEGER:
    case OT_FLOAT:
    case OT_BOOL:
    case OT_STRING:
    {
      for (const ScriptingClass* cl : parent_classes)
      {
        if (!cl) continue;

        auto it = std::find_if(cl->constants.begin(), cl->constants.end(),
                               [&base_name](const auto& func) { return func.name == base_name; });
        if (it != cl->constants.end())
        {
          object = &*it;
          break;
        }
      }
    }
    break;

    default:
      break;
  }

  // Check for match
  if (key_string == search_prefix || key_string.substr(0, search_prefix.length()) != search_prefix)
    return;

  // Push string and scripting reference object pointer
  if (remove_prefix)
  {
    const size_t pos = key_string.find_last_of('.', search_prefix.length());
    if (pos != std::string::npos)
    {
      key_string = key_string.substr(pos);
      if (key_string == ".")
        return;

      if (key_string.at(0) == '.')
        key_string.erase(0, 1);
    }
  }
  else
  {
    if (key_string.back() == '.')
      key_string.pop_back();
  }
  cmds.push_back(Suggestion(key_string, object, type == OT_INSTANCE));
}

/**
 * Calls sq_insert_command for all entries of table/class on top of stack.
 */
void
sq_insert_commands(SuggestionStack& cmds, HSQUIRRELVM vm,
                   const std::string& table_prefix, std::vector<const ScriptingClass*> parent_classes,
                   const std::string& search_prefix, bool remove_prefix)
{
  for (const ScriptingClass* parent_class : parent_classes)
  {
    if (!parent_class) continue;

    for (const std::string& base_class_name : parent_class->base_classes)
    {
      for (const auto& root : s_scripting_roots)
      {
        auto it = std::find_if(root->classes.begin(), root->classes.end(),
                               [&base_class_name](const auto& cl) { return cl.name == base_class_name; });
        if (it != root->classes.end())
        {
          parent_classes.push_back(&*it);
          break;
        }
      }
    }
  }

  sq_pushnull(vm); // Push iterator.
  while (SQ_SUCCEEDED(sq_next(vm, -2)))
  {
    sq_insert_command(cmds, vm, table_prefix, parent_classes, search_prefix, remove_prefix);
    sq_pop(vm, 2); // Pop key, value.
  }
  sq_pop(vm, 1); // Pop iterator.
}


SuggestionStack
get_suggestions(const std::string& prefix, bool remove_prefix)
{
  SuggestionStack result;
  HSQUIRRELVM vm = SquirrelVirtualMachine::current()->get_vm().get_vm();

  // Register main scripting reference data
  if (!has_registered_reference_file(SCRIPTING_REFERENCE_FILE))
    register_scripting_reference(SCRIPTING_REFERENCE_FILE);

  // Insert each scripting reference root into a pointer class
  std::vector<const ScriptingClass*> root_classes;
  for (const auto& root : s_scripting_roots)
    root_classes.push_back(root.get());

  // Append all keys of the current root table to list.
  sq_pushroottable(vm); // Push root table.
  while (true)
  {
    // Check all keys (and their children) for matches.
    sq_insert_commands(result, vm, "", root_classes, prefix, remove_prefix);

    // Cycle through parent (delegate) table.
    SQInteger oldtop = sq_gettop(vm);
    if (SQ_FAILED(sq_getdelegate(vm, -1)) || oldtop == sq_gettop(vm))
      break;

    sq_remove(vm, -2); // Remove old table.
  }
  sq_pop(vm, 1); // Pop root table.

  return result;
}

} // namespace squirrel
