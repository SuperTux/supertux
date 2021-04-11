//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_OBJECT_BRICK_HPP
#define HEADER_SUPERTUX_OBJECT_BRICK_HPP

#include "object/block.hpp"

class Brick final : public Block
{
public:
  Brick(const Vector& pos, int data, const std::string& spriteName);
  Brick(const ReaderMapping& mapping);

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;
  virtual ObjectSettings get_settings() override;
  virtual std::string get_class() const override { return "brick"; }
  virtual std::string get_display_name() const override { return _("Brick"); }

  virtual void backup(Writer& writer) const override;
  virtual void restore(const ReaderMapping& reader) override;

  void try_break(Player* player);

protected:
  virtual void hit(Player& player) override;

private:
  bool m_breakable;
  int m_coin_counter;

private:
  Brick(const Brick&) = delete;
  Brick& operator=(const Brick&) = delete;
};

#endif

/* EOF */
