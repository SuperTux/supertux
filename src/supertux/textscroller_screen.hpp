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

#ifndef HEADER_SUPERTUX_SUPERTUX_TEXTSCROLLER_SCREEN_HPP
#define HEADER_SUPERTUX_SUPERTUX_TEXTSCROLLER_SCREEN_HPP

#include <memory>

#include "supertux/screen.hpp"
#include "video/color.hpp"
#include "video/surface_ptr.hpp"

class DrawingContext;
class InfoBoxLine;

/**
 * Screen that displays intro text, extro text, etc.
 */
class TextScrollerScreen final : public Screen
{
public:
  static Color small_color;
  static Color heading_color;
  static Color reference_color;
  static Color normal_color;

public:
  TextScrollerScreen(const std::string& file);
  virtual ~TextScrollerScreen();

  virtual void setup() override;
  virtual void draw(Compositor& compositor) override;
  virtual void update(float elapsed_time) override;

  void draw(DrawingContext& context);

private:
  float m_defaultspeed;
  float m_speed;
  std::string m_music;
  SurfacePtr m_background;
  std::vector<std::unique_ptr<InfoBoxLine> > m_lines;
  float m_scroll;
  bool m_fading;

private:
  TextScrollerScreen(const TextScrollerScreen&);
  TextScrollerScreen& operator=(const TextScrollerScreen&);
};

#endif

/* EOF */
