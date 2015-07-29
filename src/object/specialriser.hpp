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

#ifndef HEADER_SUPERTUX_OBJECT_SPECIALRISER_HPP
#define HEADER_SUPERTUX_OBJECT_SPECIALRISER_HPP

#include "supertux/moving_object.hpp"

/**
 * special object that contains another object and slowly rises it out of a
 * bonus block.
 */
class SpecialRiser : public GameObject
{
public:
  SpecialRiser(Vector pos, std::shared_ptr<MovingObject> child);
  ~SpecialRiser();
  virtual bool do_save() {
    return false;
  }

  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);

private:
  float offset;
  std::shared_ptr<MovingObject> child;

private:
  SpecialRiser(const SpecialRiser&);
  SpecialRiser& operator=(const SpecialRiser&);
};

#endif

/* EOF */
