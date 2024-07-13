//  SuperTux
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
//                2024 Vankata453
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

#ifndef HEADER_SUPERTUX_UTIL_STREAM_BUFFER_HPP
#define HEADER_SUPERTUX_UTIL_STREAM_BUFFER_HPP

#include <list>
#include <sstream>
#include <string>

template<class C>
class StreamBuffer final : public std::stringbuf
{
public:
  virtual int sync() override
  {
    int result = std::stringbuf::sync();
    if (C::current())
      C::current()->flush(*this);
    return result;
  }
};

class StreamLineBuffer
{
public:
  StreamLineBuffer();
  virtual ~StreamLineBuffer();

  void add_lines(const std::string& s); /**< add (potentially) multiple lines to the backbuffer */

  void flush(std::stringbuf& buffer); /**< act upon changes in a stream buffer */

  const std::list<std::string>& get_lines() const { return m_lines; }

protected:
  virtual int add_line(const std::string& s); /**< add a line to the backbuffer */

protected:
  std::list<std::string> m_lines; /**< backbuffer of lines. New lines get added to the front. */

private:
  StreamLineBuffer(const StreamLineBuffer&) = delete;
  StreamLineBuffer& operator=(const StreamLineBuffer&) = delete;
};

#endif

/* EOF */
