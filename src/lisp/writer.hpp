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

#ifndef HEADER_SUPERTUX_LISP_WRITER_HPP
#define HEADER_SUPERTUX_LISP_WRITER_HPP

#include <string>
#include <vector>

namespace lisp {

class Writer
{
public:
  Writer(const std::string& filename);
  Writer(std::ostream* out);
  ~Writer();

  void write_comment(const std::string& comment);

  void start_list(const std::string& listname, bool string = false);

  void write(const std::string& name, int value);
  void write(const std::string& name, float value);
  void write(const std::string& name, const std::string& value,
             bool translatable = false);
  void write(const std::string& name, const char* value,
             bool translatable = false) { write(name, static_cast<const std::string&>(value), translatable); }
  void write(const std::string& name, bool value);
  void write(const std::string& name, const std::vector<int>& value);
  void write(const std::string& name, const std::vector<unsigned int>& value);
  void write(const std::string& name, const std::vector<float>& value);
  void write(const std::string& name, const std::vector<std::string>& value);
  // add more write-functions when needed...

  void end_list(const std::string& listname);

private:
  void write_escaped_string(const std::string& str);
  void indent();

private:
  std::ostream* out;
  bool out_owned;
  int indent_depth;
  std::vector<std::string> lists;

private:
  Writer(const Writer&);
  Writer & operator=(const Writer&);
};

} //namespace lisp

#endif //SUPERTUX_LISPWRITER_H

/* EOF */
