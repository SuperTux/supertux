//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef SUPERTUX_LISPWRITER_H
#define SUPERTUX_LISPWRITER_H

#include <iostream>
#include <string>
#include <vector>

namespace lisp
{

  class Writer
  {
  public:
    Writer(const std::string& filename);
    Writer(std::ostream* out);
    ~Writer();

    void write_comment(const std::string& comment);

    void start_list(const std::string& listname);

    void write_int(const std::string& name, int value);
    void write_float(const std::string& name, float value);
    void write_string(const std::string& name, const std::string& value,
        bool translatable = false);
    void write_bool(const std::string& name, bool value);
    void write_int_vector(const std::string& name, const std::vector<int>& value);
    void write_int_vector(const std::string& name, const std::vector<unsigned int>& value);
    void write_float_vector(const std::string& name, const std::vector<float>& value);
    // add more write-functions when needed...

    void end_list(const std::string& listname);

  private:
    void indent();

    std::ostream* out;
    bool out_owned;
    int indent_depth;
    std::vector<std::string> lists;
  };
  
} //namespace lisp

#endif //SUPERTUX_LISPWRITER_H

