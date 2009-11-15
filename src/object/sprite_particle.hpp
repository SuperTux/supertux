//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_OBJECT_SPRITE_PARTICLE_HPP
#define HEADER_SUPERTUX_OBJECT_SPRITE_PARTICLE_HPP


#include "supertux/game_object.hpp"
#include "supertux/resources.hpp"
#include "player.hpp"
#include "object/anchor_point.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "video/drawing_context.hpp"

class SpriteParticle : public GameObject
{
public:
  SpriteParticle(std::string sprite_name, std::string action, Vector position, AnchorPoint anchor, Vector velocity, Vector acceleration, int drawing_layer = LAYER_OBJECTS-1);
  ~SpriteParticle();
protected:
  virtual void hit(Player& player);
  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);
private:
  Sprite* sprite;
  Vector position;
  Vector velocity;
  Vector acceleration;
  int drawing_layer;

private:
  SpriteParticle(const SpriteParticle&);
  SpriteParticle& operator=(const SpriteParticle&);
};

#endif
