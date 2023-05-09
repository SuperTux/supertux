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

#include "util.hpp"

#include <sstream>
#include <fstream>

bool param_matches(int argc, char** argv, int i,
                   const std::string& rhs1, const std::string& rhs2,
                   const std::string& rhs3) // Allow for 3 parameter formats
{
  const std::string lhs = argv[i];
  return i + 1 < argc && (lhs == rhs1 || lhs == rhs2 || lhs == rhs3);
}


bool starts_with(const std::string& str, const std::string& prefix)
{
  return str.rfind(prefix, 0) == 0;
}

void replace(std::string& str, const std::string& from,
             const std::string to, const std::string to_if_empty)
{
  size_t start_pos = str.find(from);
  while (start_pos != std::string::npos)
  {
    str.replace(start_pos, from.length(), to.empty() ? to_if_empty : to);
    start_pos = str.find(from);
  }
}

void regex_replace(std::string& str, const std::regex from,
                   const std::string& to)
{
  str = std::regex_replace(str, from, to);
}


std::string read_file(const std::string& path)
{
  std::ifstream stream(path);
  std::stringstream buffer;
  buffer << stream.rdbuf();
  stream.close();
  return buffer.str();
}

void write_file(const std::string& path, const std::string& content)
{
  std::ofstream stream(path);
  stream << content;
  stream.close();
}


bool attr_equal(tinyxml2::XMLElement* el, const char* attr, const std::string& rhs)
{
  const char* val = el->FindAttribute(attr)->Value();
  return val == NULL ? rhs.empty() : std::string(val) == rhs;
}

bool el_equal(tinyxml2::XMLElement* el, const char* child_el, const std::string& rhs)
{
  const char* text = el->FirstChildElement(child_el)->GetText();
  return text == NULL ? rhs.empty() : std::string(text) == rhs;
}
