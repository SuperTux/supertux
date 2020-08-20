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

#ifndef HEADER_SUPERTUX_OBJECT_CUTSCENE_INFO_HPP
#define HEADER_SUPERTUX_OBJECT_CUTSCENE_INFO_HPP

#include "math/vector.hpp"
#include "object/camera.hpp"
#include "supertux/game_object.hpp"
#include "supertux/level.hpp"
#include "video/color.hpp"

class CutsceneInfo final : public GameObject
{
  static Color text_color;
public:
  CutsceneInfo(/*const Vector& pos*/ const Camera& cam, const std::string& text_, const Level& parent);
  virtual bool is_saveable() const override {
    return false;
  }

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;

private:
  Vector position;
  std::string text;
  const Camera& camera;
  const Level& level;
};

#endif

/* EOF */
