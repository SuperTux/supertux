//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_OBJECT_TORCH_HPP
#define HEADER_SUPERTUX_OBJECT_TORCH_HPP

#include <memory>

#include "sprite/sprite_ptr.hpp"
#include "supertux/moving_object.hpp"
#include "util/reader_fwd.hpp"

class Torch : public MovingObject
{
public:
  Torch(const Reader& reader);

  void draw(DrawingContext& context) override;
  void update(float) override;

  HitResponse collision(GameObject& other, const CollisionHit& ) override;
  virtual std::string get_class() const {
    return "torch";
  }

  virtual ObjectSettings get_settings();

private:
  SpritePtr m_torch;
  SpritePtr m_flame;
  SpritePtr m_flame_glow;
  SpritePtr m_flame_light;
  bool m_burning;

private:
  Torch(const Torch&) = delete;
  Torch& operator=(const Torch&) = delete;
};

#endif

/* EOF */
