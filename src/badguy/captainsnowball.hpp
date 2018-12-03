//  SuperTux
//  Copyright (C) 2008 Wolfgang Becker <uafr@gmx.de>
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

#ifndef HEADER_SUPERTUX_BADGUY_CAPTAINSNOWBALL_HPP
#define HEADER_SUPERTUX_BADGUY_CAPTAINSNOWBALL_HPP

#include "badguy/walking_badguy.hpp"

class CaptainSnowball final : public WalkingBadguy
{
public:
  CaptainSnowball(const ReaderMapping& reader);

  virtual void active_update(float dt_sec) override;
  virtual void collision_solid(const CollisionHit& hit) override;

  virtual std::string get_class() const override { return "captainsnowball"; }
  virtual std::string get_display_name() const override { return _("Captain snowball"); }

  bool might_climb(int width, int height) const;

protected:
  virtual bool collision_squished(GameObject& object) override;

private:
  CaptainSnowball(const CaptainSnowball&) = delete;
  CaptainSnowball& operator=(const CaptainSnowball&) = delete;
};

#endif

/* EOF */
