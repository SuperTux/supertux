//  SuperTux BadGuy GoldBomb - a bomb that throws up coins when exploding
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2013 LMH <lmh.0013@gmail.com>
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


#ifndef HEADER_SUPERTUX_BADGUY_GOLDBOMB_HPP
#define HEADER_SUPERTUX_BADGUY_GOLDBOMB_HPP

#include "badguy/mrbomb.hpp"

class SoundSource;

class GoldBomb final : public MrBomb
{
public:
  GoldBomb(const ReaderMapping& reader);

  virtual void collision_solid(const CollisionHit& hit) override;

  virtual void active_update(float dt_sec) override;

  static std::string class_name() { return "goldbomb"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Gold Bomb"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual void explode() override;

private:
  void flee(Direction dir);
  void cornered();

private:
  enum State : uint8_t {
    STATE_REALIZING = 2,
    STATE_FLEEING,
    STATE_CORNERED
  };

  Timer m_realize_timer;

private:
  GoldBomb(const GoldBomb&) = delete;
  GoldBomb& operator=(const GoldBomb&) = delete;
};

#endif

/* EOF */
