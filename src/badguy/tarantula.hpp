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

#ifndef HEADER_SUPERTUX_BADGUY_TARANTULA_HPP
#define HEADER_SUPERTUX_BADGUY_TARANTULA_HPP

#include "badguy/badguy.hpp"

class Tarantula final : public BadGuy
{
public:
  Tarantula(const ReaderMapping& reader);

  virtual void initialize() override;
  virtual void active_update(float dt_sec) override;
  virtual void collision_solid(const CollisionHit& hit) override;

  virtual void freeze() override;
  virtual void unfreeze(bool melt = true) override;
  virtual bool is_freezable() const override;

  virtual std::string get_overlay_size() const override { return "3x3"; }
  static std::string class_name() { return "tarantula"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Tarantula"); }
  virtual std::string get_display_name() const override { return display_name(); }
  virtual bool is_snipable() const override { return true; }

protected:
  virtual std::vector<Direction> get_allowed_directions() const override;

protected:
  enum TarantulaMode {
    FLY_UP,
    FLY_DOWN
  };

protected:
  virtual bool collision_squished(GameObject& object) override;

private:
  TarantulaMode mode;
  Timer timer;

private:
  Tarantula(const Tarantula&) = delete;
  Tarantula& operator=(const Tarantula&) = delete;
};

#endif

/* EOF */
