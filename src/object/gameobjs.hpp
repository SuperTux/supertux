//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef SUPERTUX_GAMEOBJS_H
#define SUPERTUX_GAMEOBJS_H

#include "video/surface.hpp"
#include "timer.hpp"
#include "game_object.hpp"
#include "moving_object.hpp"
#include "serializable.hpp"
#include "video/color.hpp"

/* Bounciness of distros: */
#define NO_BOUNCE 0
#define BOUNCE 1

class Sprite;

class BouncyCoin : public GameObject
{
public:
  BouncyCoin(const Vector& pos, bool emerge=false);
  ~BouncyCoin();
  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);

private:
  Sprite* sprite;
  Vector position;
  Timer timer;
  float emerge_distance;
};

class BrokenBrick : public GameObject
{
public:
  BrokenBrick(Sprite* sprite, const Vector& pos, const Vector& movement);
  ~BrokenBrick();

  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);

private:
  Timer timer;
  Sprite* sprite;
  Vector position;
  Vector movement;
};

class FloatingText : public GameObject
{
  static Color text_color;
public:
  FloatingText(const Vector& pos, const std::string& text_);
  FloatingText(const Vector& pos, int s);  // use this for score, for instance

  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);

private:
  Vector position;
  std::string text;
  Timer timer;
};

class SmokeCloud : public GameObject
{
public:
  SmokeCloud(const Vector& pos);
  ~SmokeCloud();

  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);

private:
  Sprite* sprite;
  Timer timer;
  Vector position;
};

#endif

/* Local Variables: */
/* mode:c++ */
/* End: */
