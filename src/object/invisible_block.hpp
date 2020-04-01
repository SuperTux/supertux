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

#ifndef HEADER_SUPERTUX_OBJECT_INVISIBLE_BLOCK_HPP
#define HEADER_SUPERTUX_OBJECT_INVISIBLE_BLOCK_HPP

#include "object/block.hpp"

class ReaderMapping;

class InvisibleBlock final : public Block
{
public:
  InvisibleBlock(const Vector& pos);
  InvisibleBlock(const ReaderMapping& mapping);

  virtual std::string get_class() const override { return "invisible_block"; }
  virtual std::string get_display_name() const override { return _("Invisible Block"); }

  virtual void draw(DrawingContext& context) override;
  virtual bool collides(GameObject& other, const CollisionHit& hit) const override;
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;

private:
  virtual void hit(Player& player) override;

private:
  bool visible;

private:
  InvisibleBlock(const InvisibleBlock&) = delete;
  InvisibleBlock& operator=(const InvisibleBlock&) = delete;
};

#endif

/* EOF */
