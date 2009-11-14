//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include <set>
#include <string>

namespace FileSystem
{
  /**
   * returns the path of the directory the file is in
   */
  std::string dirname(const std::string& filename);

  /**
   * returns the name of the file
   */
  std::string basename(const std::string& filename);

  /**
   * remove everything starting from and including the last dot
   */
  std::string strip_extension(const std::string& filename);

  /**
   * normalize filename so that "blup/bla/blo/../../bar" will become
   * "blup/bar"
   */
  std::string normalize(const std::string& filename);
}

#endif
