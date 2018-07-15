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

#include "sprite/sprite_ptr.hpp"
#include "supertux/moving_object.hpp"

class Player;
class ReaderMapping;

class Block : public MovingObject
{
public:
  Block(SpritePtr sprite);
  Block(const ReaderMapping& lisp, const std::string& sprite_file);

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;
  virtual void update(float elapsed_time) override;
  virtual void draw(DrawingContext& context) override;

  virtual std::string get_default_sprite_name() const {
    return default_sprite_name;
  }

  virtual ObjectSettings get_settings() override;
  virtual void save(Writer& writer) override;
  virtual void after_editor_set() override;

protected:
  friend class FlipLevelTransformer;

  virtual void hit(Player& player) = 0;
  void start_bounce(GameObject* hitter);
  void start_break(GameObject* hitter);
  void break_me();

  SpritePtr sprite;
  std::string sprite_name;
  std::string default_sprite_name;
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
