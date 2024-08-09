//  SuperTux
//  Copyright (C) 2023 Vankata453
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

#include "squirrel/scripting_reference.hpp"

#include <algorithm>

#include "util/log.hpp"
#include "util/reader_document.hpp"
#include "util/reader_iterator.hpp"
#include "util/reader_mapping.hpp"

namespace squirrel {

ScriptingObject::ScriptingObject(const ReaderMapping& reader) :
  name()
{
  reader.get("name", name);
  if (name.empty())
    throw std::runtime_error("No name specified.");
}

ScriptingConstant::ScriptingConstant(const ReaderMapping& reader) :
  ScriptingObject(reader),
  type(),
  description(),
  detailed_description()
{
  reader.get("type", type);
  reader.get("description", description);
  reader.get("detailed-description", detailed_description);
}

ScriptingFunction::ScriptingFunction(const ReaderMapping& reader) :
  ScriptingObject(reader),
  type(),
  description(),
  detailed_description(),
  parameters()
{
  reader.get("type", type);
  reader.get("description", description);
  reader.get("detailed-description", detailed_description);

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
  base_classes(),
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
  {
    if (iter.get_key() == "base-class")
    {
      std::string base_class_name;
      iter.get(base_class_name);
      base_classes.push_back(std::move(base_class_name));
      continue;
    }

    add_object(iter.get_key(), iter.as_mapping());
  }
}

void
ScriptingClass::add_object(const std::string& key, const ReaderMapping& reader)
{
  try
  {
    if (key == "constant")
      constants.push_back(ScriptingConstant(reader));
    else if (key == "function")
      functions.push_back(ScriptingFunction(reader));
    else if (key == "class")
      classes.push_back(ScriptingClass(reader));
  }
  catch (const std::exception& err)
  {
    log_warning << "Error adding scripting reference object '" << key << "': " << err.what() << std::endl;
  }
}

ScriptingRoot::ScriptingRoot(const std::string& file_) :
  ScriptingClass(),
  file(file_)
{
}

std::vector<std::unique_ptr<ScriptingRoot>> s_scripting_roots;

bool
has_registered_reference_file(const std::string& file)
{
  return std::any_of(s_scripting_roots.begin(), s_scripting_roots.end(),
                     [&file](const auto& root) { return root->file == file; });
}

void
register_scripting_reference(const std::string& file)
{
  if (has_registered_reference_file(file))
  {
    log_warning << "Cannot register scripting reference data twice from the same file: '" << file << "'." << std::endl;
    return;
  }

  try
  {
    auto scripting_root = std::make_unique<ScriptingRoot>(file);

    auto doc = ReaderDocument::from_file(file);
    auto root = doc.get_root();
    if (root.get_name() == "supertux-scripting-reference")
    {
      auto iter = root.get_mapping().get_iter();
      while (iter.next())
        scripting_root->add_object(iter.get_key(), iter.as_mapping());
    }
    else
    {
      throw std::runtime_error("File is not a 'supertux-scripting-reference' file.");
    }

    s_scripting_roots.push_back(std::move(scripting_root));
  }
  catch (const std::exception& err)
  {
    log_warning << "Cannot load scripting reference data from '" << file << "': " << err.what() << std::endl;
  }
}

void
unregister_scripting_reference(const std::string& file)
{
  s_scripting_roots.erase(
    std::remove_if(s_scripting_roots.begin(), s_scripting_roots.end(),
      [&file](const auto& root) { return root->file == file; }),
    s_scripting_roots.end());
}

} // namespace squirrel
