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

#ifndef HEADER_SUPERTUX_PHYSFS_IFILE_STREAMBUF_HPP
#define HEADER_SUPERTUX_PHYSFS_IFILE_STREAMBUF_HPP

#include <streambuf>
#include <physfs.h>

/** This class implements a C++ streambuf object for physfs files.
 * So that you can use normal istream operations on them
 */
class IFileStreambuf : public std::streambuf
{
public:
  IFileStreambuf(const std::string& filename);
  ~IFileStreambuf();

protected:
  virtual int underflow();
  virtual pos_type seekoff(off_type pos, std::ios_base::seekdir,
                           std::ios_base::openmode);
  virtual pos_type seekpos(const pos_type& pos, std::ios_base::openmode);

private:
  PHYSFS_file* file;
  char buf[1024];

private:
  IFileStreambuf(const IFileStreambuf&);
  IFileStreambuf& operator=(const IFileStreambuf&);
};

#endif

/* EOF */
