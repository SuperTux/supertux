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

#ifndef HEADER_SUPERTUX_OBJECT_SPOTLIGHT_HPP
#define HEADER_SUPERTUX_OBJECT_SPOTLIGHT_HPP

#include <memory>

#include "util/reader_fwd.hpp"
#include "math/vector.hpp"
#include "sprite/sprite_ptr.hpp"
#include "supertux/game_object.hpp"
#include "video/color.hpp"

class Spotlight : public GameObject
{
public:
  Spotlight(const Reader& reader);
  virtual ~Spotlight();
  virtual void save(lisp::Writer& writer);

  void update(float elapsed_time);
  void draw(DrawingContext& context);
  virtual std::string get_class() {
    return "spotlight";
  }

  virtual ObjectSettings get_settings();

private:
  Vector  position;
  float   angle;
  SpritePtr center;
  SpritePtr base;
  SpritePtr lights;
  SpritePtr light;
  SpritePtr lightcone;

  Color   color;
};

#endif

/* EOF */
