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

#ifndef HEADER_SUPERTUX_BADGUY_SNOWBALL_HPP
#define HEADER_SUPERTUX_BADGUY_SNOWBALL_HPP

#include "badguy/walking_badguy.hpp"

class SnowBall final : public WalkingBadguy
{
public:
  SnowBall(const ReaderMapping& reader);
  SnowBall(const Vector& pos, Direction d, const std::string& script);

  static std::string class_name() { return "snowball"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Snowball"); }
  virtual std::string get_display_name() const override { return display_name(); }

protected:
  virtual bool collision_squished(GameObject& object) override;

private:
  SnowBall(const SnowBall&) = delete;
  SnowBall& operator=(const SnowBall&) = delete;
};

#endif

/* EOF */
