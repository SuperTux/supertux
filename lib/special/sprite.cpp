//  $Id$
//
//  SuperTux
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
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

#include <iostream>
#include <cmath>

#include "../app/globals.h"
#include "../app/setup.h"
#include "../special/sprite.h"
#include "../video/drawing_context.h"

using namespace SuperTux;

Sprite::Sprite(lisp_object_t* cur)
{
  for(; !lisp_nil_p(cur); cur = lisp_cdr(cur))
    {
    std::string token = lisp_symbol(lisp_car(lisp_car(cur)));
    lisp_object_t* data = lisp_car(lisp_cdr(lisp_car(cur)));
    LispReader reader(lisp_cdr(lisp_car(cur)));

    if(token == "name")
      name = lisp_string(data);
    else if(token == "action")
      parse_action(reader);
    else
      std::cerr << "Warning: Unknown sprite field: " << token << std::endl;
    }

  if(name.empty())
    Termination::abort("Error: Sprite wihtout name.", "");
  if(actions.empty())
    Termination::abort("Error: Sprite wihtout actions.", "");
}

Sprite::~Sprite()
{
  for(Actions::iterator i_act = actions.begin(); i_act != actions.end(); ++i_act)
    {
    for(std::vector<Surface*>::iterator i_sur = i_act->second->surfaces.begin();
        i_sur != i_act->second->surfaces.end(); ++i_sur)
      delete *i_sur;
    delete i_act->second;
    }
}

void
Sprite::parse_action(LispReader& lispreader)
{
  action = new Action;

  init_defaults(action);

  if(!lispreader.read_string("name", action->name))
    if(!actions.empty())
      Termination::abort("Error: If there are more than one action, they need names!", "");
  lispreader.read_int("x-offset", action->x_offset);
  lispreader.read_int("y-offset", action->y_offset);
  lispreader.read_int("z-order", action->z_order);
  lispreader.read_float("fps",     action->fps);

  std::vector<std::string> images;
  if(!lispreader.read_string_vector("images", images))
    Termination::abort("Sprite contains no images: ", action->name.c_str());

  for(std::vector<std::string>::size_type i = 0; i < images.size(); ++i)
    {
      action->surfaces.push_back(
          new Surface(datadir + "/images/" + images[i], true));
    }        

  actions[action->name] = action;
}

void
Sprite::init_defaults(Action* act)
{
  act->x_offset = 0;
  act->y_offset = 0;
  act->z_order = 0;
  act->fps = 10;

  start_animation(-1);
}

void
Sprite::set_action(std::string act)
{
if(!next_action.empty() && animation_loops > 0)
  {
  next_action = act;
  return;
  }
Actions::iterator i = actions.find(act);
if(i == actions.end())
  {
  std::cerr << "Warning: Action '" << act << "' not found on Sprite '" << name << "'\n";
  return;
  }
action = i->second;
}

void
Sprite::start_animation(int loops)
{
reset();
animation_loops = loops;
}

void
Sprite::reset()
{
frame = 0;
last_tick = SDL_GetTicks();
animation_reversed = false;
next_action.clear();
}

bool
Sprite::check_animation()
{
return animation_loops;
}

void
Sprite::reverse_animation(bool reverse)
{
animation_reversed = reverse;

if(animation_reversed)
  frame = get_frames()-1;
else
  frame = 0;
}

void
Sprite::update()
{
if(animation_loops == 0)
  return;

float frame_inc = (action->fps/1000.0) * (SDL_GetTicks() - last_tick);
last_tick = SDL_GetTicks();

if(animation_reversed)
  frame -= frame_inc;
else
  frame += frame_inc;

if(animation_reversed)
  {
  float excedent = frame - 0;
  if((int)excedent < 0 || excedent >= get_frames())
    {  // last case can happen when not used reverse_animation()
    frame = get_frames() - 1;
    if(animation_loops > 0)
      {
      animation_loops--;
      if(animation_loops == 0 && !next_action.empty())
        {
        set_action(next_action);
        start_animation(-1);
        }
      }

    if(fabsf(excedent) < get_frames())
      frame += excedent;
    }
  }
else
  {
  float excedent = frame - action->surfaces.size();
  if((int)excedent >= 0)
    {
    frame = 0;
    if(animation_loops > 0)
      {
      animation_loops--;
      if(animation_loops == 0 && !next_action.empty())
        {
        set_action(next_action);
        start_animation(-1);
        }
      }

    if(excedent < get_frames())
      frame += excedent;
    }
  }
}

void
Sprite::draw(DrawingContext& context, const Vector& pos, int layer,
    Uint32 drawing_effect)
{
  update();

  if((int)frame >= get_frames() || (int)frame < 0)
    std::cerr << "Warning: frame out of range: " << (int)frame
              << "/" << get_frames() << " at sprite: " << get_name()
              << "/" << get_action_name() << std::endl;
  else
    context.draw_surface(action->surfaces[(int)frame],
            pos - Vector(action->x_offset, action->y_offset), layer + action->z_order,
            drawing_effect);
}

void
Sprite::draw_part(DrawingContext& context, const Vector& source, const Vector& size,
                  const Vector& pos, int layer, Uint32 drawing_effect)
{
  update();

  if((int)frame >= get_frames() || (int)frame < 0)
    std::cerr << "Warning: frame out of range: " << (int)frame
              << "/" << get_frames() << " at sprite: " << get_name()
              << "/" << get_action_name() << std::endl;
  else
    context.draw_surface_part(action->surfaces[(int)frame], source, size,
            pos - Vector(action->x_offset, action->y_offset), layer + action->z_order,
            drawing_effect);
}

int
Sprite::get_width()
{
  return action->surfaces[get_frame()]->w;
}

int
Sprite::get_height()
{
  return action->surfaces[get_frame()]->h;
}

/* EOF */
