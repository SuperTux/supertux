//  Copyright (C) 2020 Daniel Ward <weluvgoatz@gmail.com>
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

#ifndef HEADER_SUPERTUX_OBJECT_BUMPER_HPP
#define HEADER_SUPERTUX_OBJECT_BUMPER_HPP

#include "object/moving_sprite.hpp"
#include "supertux/physic.hpp"

class Player;

class Bumper final : public MovingSprite
{
public:
  Bumper(const ReaderMapping& reader);

  virtual ObjectSettings get_settings() override;
  
  virtual void update(float dt_sec) override;
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;
  
  virtual std::string get_class() const override { return "bumper"; }
  virtual std::string get_display_name() const override { return _("Bumper"); }

  virtual void backup(Writer& writer) const override;
  virtual void restore(const ReaderMapping& reader) override;

  Physic physic;

private:
  bool left;
	
private:
  Bumper(const Bumper&) = delete;
  Bumper& operator=(const Bumper&) = delete;
};

#endif

/* EOF */
