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

#include <algorithm>
#include <optional>

#include "squirrel/squirrel_virtual_machine.hpp"
#include "util/log.hpp"
#include "util/reader_document.hpp"
#include "util/reader_iterator.hpp"
#include "util/reader_mapping.hpp"

namespace squirrel {

static std::optional<ScriptingClass> s_scripting_root;

namespace {

void sq_insert_commands(SuggestionStack& cmds, HSQUIRRELVM vm,
                        const std::string& table_prefix, const ScriptingClass* parent_class,
                        const std::string& search_prefix, bool remove_prefix);

/**
 * Acts upon key, value on top of stack:
 * Appends key (plus type-dependent suffix) to cmds if table_prefix+key starts with search_prefix;
 * Calls sq_insert_commands if search_prefix starts with table_prefix+key (and value is a table/class/instance);
 */
void
sq_insert_command(SuggestionStack& cmds, HSQUIRRELVM vm,
                  const std::string& table_prefix, const ScriptingClass* parent_class,
                  const std::string& search_prefix, bool remove_prefix)
{
  const SQChar* key_chars;
  if (SQ_FAILED(sq_getstring(vm, -2, &key_chars))) return;
  std::string base_name = key_chars;
  std::string key_string = table_prefix + base_name;

  const ScriptingObject* object = nullptr;

  switch (sq_gettype(vm, -1))
  {
    case OT_INSTANCE:
      key_string += ".";
      if (search_prefix.substr(0, key_string.length()) == key_string)
      {
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
              if (SQ_FAILED(sq_getstring(vm, -3, &key_chars))) return;
              found_class = true;
            }
            sq_pop(vm, 1); // Pop instance.
          }
          sq_pop(vm, 2); // Pop key, value.
        }
        sq_pop(vm, 2); // Pop iterator, root table.

        const ScriptingClass* current_class = nullptr;
        if (found_class) // Succeeded getting class name
        {
          base_name = key_chars; // Use class name

          auto it = std::find_if(s_scripting_root->classes.begin(), s_scripting_root->classes.end(),
                                 [&base_name](const auto& cl) { return cl.name == base_name; });
          if (it != s_scripting_root->classes.end())
          {
            current_class = &*it;
            object = current_class;
          }
        }

        sq_getclass(vm, -1); // Push class.
        sq_insert_commands(cmds, vm, key_string, current_class, search_prefix, remove_prefix);
        sq_pop(vm, 1); // Pop class.
      }
      break;

    case OT_TABLE:
    case OT_CLASS:
      key_string += ".";
      if (search_prefix.substr(0, key_string.length()) == key_string)
      {
        const ScriptingClass* current_class = nullptr;
        auto it = std::find_if(s_scripting_root->classes.begin(), s_scripting_root->classes.end(),
                               [&base_name](const auto& cl) { return cl.name == base_name; });
        if (it != s_scripting_root->classes.end())
        {
          current_class = &*it;
          object = current_class;
        }

        sq_insert_commands(cmds, vm, key_string, current_class, search_prefix, remove_prefix);
      }
      break;

    case OT_CLOSURE:
      key_string += "()";
      break;
    case OT_NATIVECLOSURE:
      key_string += "()";
      if (parent_class)
      {
        auto it = std::find_if(parent_class->functions.begin(), parent_class->functions.end(),
                               [&base_name](const auto& func) { return func.name == base_name; });
        if (it != parent_class->functions.end())
          object = &*it;
      }
      break;

    case OT_INTEGER:
    case OT_FLOAT:
    case OT_BOOL:
    case OT_STRING:
      if (parent_class)
      {
        auto it = std::find_if(parent_class->constants.begin(), parent_class->constants.end(),
                               [&base_name](const auto& func) { return func.name == base_name; });
        if (it != parent_class->constants.end())
          object = &*it;
      }
      break;

    default:
      break;
  }

  // Check for match
  if (key_string == search_prefix || key_string.substr(0, search_prefix.length()) != search_prefix)
    return;

  // Push string and object pointer
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

        cmds.insert({ result, object }); // TEMP
      }
      return;
    }
  }
  cmds.insert({ key_string, object }); // TEMP
}

/**
 * Calls sq_insert_command for all entries of table/class on top of stack.
 */
void
sq_insert_commands(SuggestionStack& cmds, HSQUIRRELVM vm,
                   const std::string& table_prefix, const ScriptingClass* parent_class,
                   const std::string& search_prefix, bool remove_prefix)
{
  sq_pushnull(vm); // Push iterator.
  while (SQ_SUCCEEDED(sq_next(vm, -2)))
  {
    sq_insert_command(cmds, vm, table_prefix, parent_class, search_prefix, remove_prefix);
    sq_pop(vm, 2); // Pop key, value.
  }
  sq_pop(vm, 1); // Pop iterator.
}

} // namespace


ScriptingObject::ScriptingObject(const ReaderMapping& reader) :
  name()
{
  reader.get("name", name);
}

ScriptingConstant::ScriptingConstant(const ReaderMapping& reader) :
  ScriptingObject(reader),
  type(),
  description()
{
  reader.get("type", type);
  reader.get("description", description);
}

ScriptingFunction::ScriptingFunction(const ReaderMapping& reader) :
  ScriptingObject(reader),
  type(),
  description(),
  parameters()
{
  reader.get("type", type);
  reader.get("description", description);

  auto iter = reader.get_iter();
  while (iter.next())
  {
    if (iter.get_key() != "parameter")
      continue;

    auto param_reader = iter.as_mapping();

    Parameter param;
    param_reader.get("type", param.type);
    param_reader.get("name", param.name);
    param_reader.get("description", param.description);

    parameters.push_back(std::move(param));
  }
}

ScriptingClass::ScriptingClass(const ReaderMapping& reader) :
  ScriptingObject(reader),
  summary(),
  instances(),
  constants(),
  functions(),
  classes()
{
  reader.get("summary", summary);
  reader.get("instances", instances);

  auto iter = reader.get_iter();
  while (iter.next())
    add_object(iter.get_key(), iter.as_mapping());
}

void
ScriptingClass::add_object(const std::string& key, const ReaderMapping& reader)
{
  if (key == "constant")
    constants.push_back(ScriptingConstant(reader));
  else if (key == "function")
    functions.push_back(ScriptingFunction(reader));
  else if (key == "class")
    classes.push_back(ScriptingClass(reader));
}


SuggestionStack
autocomplete(const std::string& prefix, bool remove_prefix)
{
  SuggestionStack result;
  HSQUIRRELVM vm = SquirrelVirtualMachine::current()->get_vm().get_vm();

  // Parse scripting reference data
  if (!s_scripting_root)
  {
    // Create global objects class
    s_scripting_root = ScriptingClass();

    auto doc = ReaderDocument::from_file("scripts/reference.stsr");
    auto root = doc.get_root();
    if (root.get_name() == "supertux-scripting-reference")
    {
      auto iter = root.get_mapping().get_iter();
      while (iter.next())
        s_scripting_root->add_object(iter.get_key(), iter.as_mapping());
    }
    else
    {
      log_warning << "Cannot load scripting reference data: 'scripts/reference.stsr' is not a 'supertux-scripting-reference' file." << std::endl;
    }
  }

  // Append all keys of the current root table to list.
  sq_pushroottable(vm); // Push root table.
  while (true)
  {
    // Check all keys (and their children) for matches.
    sq_insert_commands(result, vm, "", &*s_scripting_root, prefix, remove_prefix);

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
