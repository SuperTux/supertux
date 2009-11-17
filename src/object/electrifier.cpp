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

#include "object/electrifier.hpp"
#include "supertux/sector.hpp"

Electrifier::Electrifier(uint32_t oldtile, uint32_t newtile, float seconds)
{
  duration.start(seconds);
  change_from = oldtile;
  change_to = newtile;
  Sector::current()->change_solid_tiles(change_from,change_to);
}

Electrifier::~Electrifier() {
}

void
Electrifier::update(float )
{
  if (duration.check()) {
    Sector::current()->change_solid_tiles(change_to,change_from);
    remove_me();
  }
}

void
Electrifier::draw(DrawingContext& )
{
}

/* EOF */
