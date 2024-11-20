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

#include "util/stream_buffer.hpp"

#include "video/font.hpp"

StreamLineBuffer::StreamLineBuffer() :
  m_lines()
{
}

StreamLineBuffer::~StreamLineBuffer()
{
}

void
StreamLineBuffer::add_lines(const std::string& s)
{
  std::istringstream iss(s);
  std::string line;
  while (std::getline(iss, line, '\n'))
  {
    add_line(line);
  }
}

int
StreamLineBuffer::add_line(const std::string& s_)
{
  std::string s = s_;

  // Wrap long lines.
  std::string overflow;
  int line_count = 0;
  do
  {
    m_lines.push_front(Font::wrap_to_chars(s, 99, &overflow));
    line_count += 1;
    s = overflow;
  } while (s.length() > 0);

  // Trim scrollback buffer.
  while (m_lines.size() >= 1000)
  {
    m_lines.pop_back();
  }

  return line_count;
}

void
StreamLineBuffer::flush(std::stringbuf& buffer)
{
  std::string s = buffer.str();
  if ((s.length() > 0) && ((s[s.length()-1] == '\n') || (s[s.length()-1] == '\r')))
  {
    while ((s[s.length()-1] == '\n') || (s[s.length()-1] == '\r'))
    {
      s.erase(s.length()-1);
    }
    add_lines(s);
    buffer.str("");
  }
}

/* EOF */
