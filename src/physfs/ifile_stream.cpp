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

#include "physfs/ifile_stream.hpp"

#include "physfs/ifile_streambuf.hpp"

namespace {
    IFileStreambuf* create_in_buffer(const std::string& filename)
    {
        auto streambuf = new IFileStreambuf(filename);
        if(streambuf != NULL)
        {
            return streambuf;
        }

        delete streambuf;
        return NULL;
    }
}

IFileStream::IFileStream(const std::string& filename) :
  std::istream(create_in_buffer(filename))
{
}

IFileStream::~IFileStream()
{
  delete rdbuf();
}

/* EOF */
