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

#ifndef HEADER_SUPERTUX_OBJECT_BLOCK_HPP
#define HEADER_SUPERTUX_OBJECT_BLOCK_HPP

#include <memory>

#include "sprite/sprite_ptr.hpp"
#include "supertux/moving_object.hpp"
#include "util/reader_fwd.hpp"

class Player;

class Block : public MovingObject
{
public:
  Block(SpritePtr sprite);
  Block(const Reader& lisp, std::string sprite_file);
  ~Block();

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);
  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);

protected:
  friend class FlipLevelTransformer;

  virtual void hit(Player& player) = 0;
  void start_bounce(GameObject* hitter);
  void start_break(GameObject* hitter);
  void break_me();

  SpritePtr sprite;
  bool bouncing;
  bool breaking;
  float bounce_dir;
  float bounce_offset;
  float original_y;

private:
  Block(const Block&);
  Block& operator=(const Block&);
};

#endif

/* EOF */
