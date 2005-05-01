//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2000 Bill Kendrick <bill@newbreedsoftware.com>
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

#ifndef SUPERTUX_SETUP_H
#define SUPERTUX_SETUP_H

#include <vector>
#include <set>
#include <string>

namespace SuperTux {

/// File system utility functions
struct FileSystem
  {
    static bool faccessible(const std::string& filename);
    static bool fcreatedir(const std::string& relative_dir);
    static bool fwriteable(const std::string& filename);
    static std::set<std::string> read_directory(const std::string& pathname);
    static std::set<std::string> dsubdirs(const std::string& rel_path, const std::string& expected_file);
    static std::set<std::string> dfiles(const std::string& rel_path, const std::string& glob, const std::string& exception_str);

    static std::string dirname(const std::string& filename);
  };

} //namespace SuperTux

#endif /*SUPERTUX_SETUP_H*/

