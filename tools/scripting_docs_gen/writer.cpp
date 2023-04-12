//  SuperTux - Scripting reference generator
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

#include "writer.hpp"

#include <sstream>

#include "util.hpp"

namespace Writer {

std::string write_file_notice(const std::string& template_file)
{
  std::stringstream notice;

  notice << "> Note: This file is auto-generated from the [SuperTux scripting interface source code](https://github.com/SuperTux/supertux/tree/master/src/scripting), "
         << "using the template [" << template_file << "](https://github.com/SuperTux/wiki/tree/master/templates/" << template_file << ")."
         << std::endl << std::endl;

  return notice.str();
}


std::string write_constants_table(const std::vector<Constant>& constants)
{
  if (constants.empty()) return "";

  std::stringstream table;

  // Table beginning
  table << "Constant | Explanation" << std::endl;
  table << "---------|---------" << std::endl;
  // Table contents (constants)
  for (const Constant& con : constants)
  {
    // Print out type, name and description
    table << "`" << con.type << " " << con.name << "` | " << con.description << std::endl;
  }

  return table.str();
}

std::string write_function_table(const std::vector<Function>& functions)
{
  if (functions.empty()) return "";

  std::stringstream table;

  // Table beginning
  table << "Method | Explanation" << std::endl;
  table << "-------|-------" << std::endl;
  // Table contents (functions)
  for (const Function& func : functions)
  {
    // Print out function
    table << "`" << func.name << "(";
    for (size_t i = 0; i < func.parameters.size(); i++)
    {
      if (i != 0) table << ", ";
      table << func.parameters[i].type << " " << func.parameters[i].name;
    }
    table << ")`";

    // Print out description of function
    table << " | " << func.description;

    // Print out descriptions of parameters
    bool has_printed_param_desc = false;
    for (const Parameter& param : func.parameters)
    {
      if (param.description.empty()) continue;
      table << (has_printed_param_desc ? "" : "<br />") << "<br /> `" << param.name << "` - " << param.description;
      has_printed_param_desc = true;
    }

    // End table entry
    table << std::endl;
  }

  return table.str();
}

std::string write_class_list(const std::vector<Class>& classes)
{
  std::stringstream list;

  // For each class name, create an list entry
  for (const Class& cl : classes)
  {
    list << "* " << write_class_ref(cl.name) << std::endl;
  }

  return list.str();
}


std::string write_class_ref(const std::string& name)
{
  return "[" + name + "](https://github.com/SuperTux/supertux/wiki/Scripting" + name + ")";
}

} // namespace Writer
