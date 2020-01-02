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

#ifndef HEADER_SUPERTUX_BADGUY_MRTREE_HPP
#define HEADER_SUPERTUX_BADGUY_MRTREE_HPP

#include "badguy/walking_badguy.hpp"

class MrTree final : public WalkingBadguy
{
public:
  MrTree(const ReaderMapping& reader);

  virtual bool is_freezable() const override;
  virtual std::string get_class() const override { return "mrtree"; }
  virtual std::string get_display_name() const override { return _("Walking Tree"); }

protected:
  virtual bool collision_squished(GameObject& object) override;

private:
  MrTree(const MrTree&) = delete;
  MrTree& operator=(const MrTree&) = delete;
};

#endif

/* EOF */
