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

#ifndef HEADER_SUPERTUX_SUPERTUX_TEXTSCROLLER_HPP
#define HEADER_SUPERTUX_SUPERTUX_TEXTSCROLLER_HPP

#include <map>
#include <memory>

#include "supertux/screen.hpp"
#include "video/color.hpp"
#include "video/surface_ptr.hpp"

class DrawingContext;
class Font;
class InfoBoxLine;

/**
 * Screen that displays intro text, extro text, etc.
 */
class TextScroller : public Screen
{
public:
  TextScroller(const std::string& file);
  virtual ~TextScroller();

  void setup();
  void draw(DrawingContext& context);
  void update(float elapsed_time);

  static Color small_color;
  static Color heading_color;
  static Color reference_color;
  static Color normal_color;

private:
  float defaultspeed;
  float speed;
  std::string music;
  SurfacePtr background;
  std::vector<std::unique_ptr<InfoBoxLine> > lines;
  float scroll;
  bool fading;

private:
  TextScroller(const TextScroller&);
  TextScroller& operator=(const TextScroller&);
};

#endif

/* EOF */
