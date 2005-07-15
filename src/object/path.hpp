//  $Id:$
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
#ifndef __PATH_HPP__
#define __PATH_HPP__

#include <string>
#include <list>
#include <map>

#include "math/vector.hpp"
#include "game_object.hpp"
#include "lisp/lisp.hpp"


class   Path;
typedef std::map<std::string,Path*>       PathRegistry;


typedef std::list<Vector>                 PathPoints;
typedef std::list<Vector>::const_iterator PathPointIter;


class Path : public GameObject
{
public:
  Path(const lisp::Lisp& reader);
  ~Path();

  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);

  const Vector& GetPosition();
  const Vector& GetStart();
  const Vector& GetLastMovement();

  // WARNING: returns NULL if not found !
  static Path* GetByName(const std::string& name);

private:
  std::string   name;
  float         pixels_per_second;
  PathPoints    points;	
  PathPointIter next_target;
  Vector        pos;
  Vector        velocity;
  Vector        last_movement;

  void calc_next_velocity();

  static PathRegistry registry;
};

#endif
