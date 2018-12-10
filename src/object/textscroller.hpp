//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2018 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_OBJECT_TEXTSCROLLER_HPP
#define HEADER_SUPERTUX_OBJECT_TEXTSCROLLER_HPP

#include <memory>
#include <vector>

#include "supertux/info_box_line.hpp"
#include "supertux/game_object.hpp"

class DrawingContext;
class InfoBoxLine;
class ReaderCollection;
class ReaderMapping;
class ReaderObject;

class TextScroller : public GameObject
{
public:
  TextScroller(const ReaderMapping& mapping);
  TextScroller(const ReaderObject& root);

  virtual void draw(DrawingContext& context) override;
  virtual void update(float dt_sec) override;
  virtual ObjectSettings get_settings() override;
  virtual std::string get_class() const override { return "textscroller"; }
  virtual std::string get_display_name() const override { return _("TextScroller"); }

  void set_speed(float speed);
  void scroll(float offset);
  bool is_finished() const { return m_finished; }

private:
  void parse_file(const std::string& filename);
  void parse_root(const ReaderObject& root);
  void parse_content(const ReaderCollection& collection);

private:
  std::string m_filename;
  std::vector<std::unique_ptr<InfoBoxLine> > m_lines;
  float m_scroll;
  float m_speed;
  bool m_finished;

private:
  TextScroller(const TextScroller&) = delete;
  TextScroller& operator=(const TextScroller&) = delete;
};

#endif

/* EOF */
