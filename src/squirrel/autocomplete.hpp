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

#ifndef HEADER_SUPERTUX_SQUIRREL_AUTOCOMPLETE_HPP
#define HEADER_SUPERTUX_SQUIRREL_AUTOCOMPLETE_HPP

#include <map>
#include <string>
#include <vector>

class ReaderMapping;

namespace squirrel {

struct ScriptingObject
{
  ScriptingObject() = default;
  ScriptingObject(const ReaderMapping& reader);

  enum class Type { CONSTANT, FUNCTION, CLASS };
  virtual Type get_type() const = 0;

  std::string name;
};

struct ScriptingConstant final : public ScriptingObject
{
  ScriptingConstant(const ReaderMapping& reader);

  Type get_type() const override { return Type::CONSTANT; }

  std::string type;
  std::string description;
};

struct ScriptingFunction final : public ScriptingObject
{
  ScriptingFunction(const ReaderMapping& reader);

  Type get_type() const override { return Type::FUNCTION; }

  struct Parameter final
  {
    Parameter() = default;

    std::string type;
    std::string name;
    std::string description;
  };

  std::string type;
  std::string description;
  std::vector<Parameter> parameters;
};

struct ScriptingClass final : public ScriptingObject
{
  ScriptingClass() = default;
  ScriptingClass(const ReaderMapping& reader);

  void add_object(const std::string& key, const ReaderMapping& reader);

  Type get_type() const override { return Type::CLASS; }

  std::string summary;
  std::string instances;
  std::vector<ScriptingConstant> constants;
  std::vector<ScriptingFunction> functions;
  std::vector<ScriptingClass> classes;
};

typedef std::map<std::string, const ScriptingObject*> SuggestionStack;

SuggestionStack autocomplete(const std::string& prefix, bool remove_prefix);

} // namespace squirrel

#endif
