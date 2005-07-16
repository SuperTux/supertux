//  $Id$
// 
//  SuperTux
//  Copyright (C) 2005 Philipp <balinor@pnxs.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#include "path.hpp"

#include "lisp/lisp.hpp"
#include "lisp/list_iterator.hpp"
#include "object_factory.hpp"

#include <assert.h>


// some constants
#define DEFAULT_PIXELS_PER_SECOND	50
#define EPSILON				1.5

Path::Path(const lisp::Lisp& reader)
{
  forward = true;
  float x,y;

  lisp::ListIterator iter(&reader);

  assert (iter.next());
  std::string token = iter.item();
  assert(token == "name");
  iter.value()->get(name);

  circular = true;
  assert (iter.next());
  token = iter.item();
  if (token == "circular") {
    iter.value()->get(circular);
    iter.next();
  }

  pixels_per_second = DEFAULT_PIXELS_PER_SECOND;
  assert (iter.next());
  token = iter.item();
  if (token == "speed") {
    iter.value()->get(pixels_per_second);
    iter.next();
  }
  do {
    token = iter.item();
    if(token == "x") {
      iter.value()->get(x);
    } else if(token == "y") {
      iter.value()->get(y);
      points.push_back(Vector(x,y));
    }
  } while(iter.next());

  next_target = points.begin();
  pos = *next_target;

  calc_next_velocity();

  // register this path for lookup:
  registry[name] = this;
}

Path::~Path()
{
  registry.erase(name);
}

void
Path::update(float elapsed_time)
{
  last_movement = velocity * elapsed_time;
  pos += last_movement;
  if ((pos - *next_target).norm() < EPSILON) {
    pos = *next_target;
    calc_next_velocity();
  }
}

void
Path::draw(DrawingContext& context)
{
   // TODO: Add a visible flag, draw the path if true
}

const Vector&
Path::GetPosition() {
  return pos;
}

const Vector&
Path::GetStart() {
  return *(points.begin());
}

const Vector&
Path::GetLastMovement() {
  return last_movement;
}

void
Path::calc_next_velocity()
{
  Vector distance;

  if (circular) {
    ++next_target;
    if (next_target == points.end()) {
      next_target = points.begin();
    }
  }
  else if (forward) {
    ++next_target;
    if (next_target == points.end()) {
      forward = false;
    }
  }
  else {
    //FIXME: Implement going backwards on the list
    //       I have no f***ing idea how this is done in C++
  }

  distance = *next_target - pos;
  velocity = distance.unit() * pixels_per_second;
}

//////////////////////////////////////////////////////////////////////////////
// static stuff

PathRegistry Path::registry;

Path*
Path::GetByName(const std::string& name) {
  return registry[name];
}

IMPLEMENT_FACTORY(Path, "path");
