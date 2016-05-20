//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2016 M. Teufel <mteufel@urandom.eu.org>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_CREDITSSCROLLER_HPP
#define HEADER_SUPERTUX_SUPERTUX_CREDITSSCROLLER_HPP

#include <map>
#include <memory>

#include "supertux/screen.hpp"
#include "video/color.hpp"
#include "video/surface_ptr.hpp"

class DrawingContext;

/**
 * Screen that displays intro text, extro text, etc.
 */
class CreditsScroller : public Screen
{
public:
  CreditsScroller(const std::string& file);
  virtual ~CreditsScroller();

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
  float scroll;
  bool fading;

private:
  CreditsScroller(const CreditsScroller&);
  CreditsScroller& operator=(const CreditsScroller&);
};

#endif

/* EOF */
