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

#ifndef HEADER_SUPERTUX_PHYSFS_OFILE_STREAMBUF_HPP
#define HEADER_SUPERTUX_PHYSFS_OFILE_STREAMBUF_HPP

#include <streambuf>
#include <physfs.h>

class OFileStreambuf : public std::streambuf
{
public:
  OFileStreambuf(const std::string& filename);
  ~OFileStreambuf();

protected:
  virtual int overflow(int c);
  virtual int sync();

private:
  PHYSFS_file* file;
  char buf[1024];

private:
  OFileStreambuf(const OFileStreambuf&);
  OFileStreambuf& operator=(const OFileStreambuf&);
};

#endif

/* EOF */
