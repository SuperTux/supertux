//  SuperTux
//  Copyright (C) 2010 Florian Forster <supertux at octo.it>
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

#ifndef HEADER_SUPERTUX_BADGUY_BOMBFISH_HPP
#define HEADER_SUPERTUX_BADGUY_BOMBFISH_HPP

#include "badguy/badguy.hpp"
#include "object/portable.hpp"

class SkyDive : public BadGuy, public Portable
{
  private:
    bool is_grabbed;

  public:
    SkyDive(const ReaderMapping& reader);

    void collision_solid(const CollisionHit& hit);
    HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit);
    void collision_tile(uint32_t tile_attributes);

    /* Inherited from Portable */
    void grab(MovingObject& object, const Vector& pos, Direction dir);
    void ungrab(MovingObject& object, Direction dir);
    std::string get_class() const {
      return "skydive";
    }
    std::string get_display_name() const {
      return _("Sky dive");
    }

  protected:
    HitResponse collision_player(Player& player, const CollisionHit& hit);
    bool collision_squished (GameObject& obj);

    void active_update (float elapsed_time);

    void explode();
};

#endif /* HEADER_SUPERTUX_BADGUY_BOMBFISH_HPP */

/* vim: set sw=2 sts=2 et fdm=marker : */
/* EOF */
