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
  lispreader.read_int("x-hotspot", action->x_hotspot);
  lispreader.read_int("y-hotspot", action->y_hotspot);
  lispreader.read_float("fps",     action->fps);

  std::vector<std::string> images;
  if(!lispreader.read_string_vector("images", images))
    Termination::abort("Sprite contains no images: ", action->name.c_str());

  for(std::vector<std::string>::size_type i = 0; i < images.size(); ++i)
    {
      action->surfaces.push_back(
          new Surface(datadir + "/images/" + images[i], true));
    }        

  action->frame_delay = 1000.0f/action->fps;

  actions[action->name] = action;
}

void
Sprite::init_defaults(Action* act)
{
  act->x_hotspot = 0;
  act->y_hotspot = 0;
  act->fps = 10;
  act->frame_delay = 1000.0f/act->fps;
  time = 0;
}

void
Sprite::set_action(std::string& act)
{
Actions::iterator i = actions.find(act);
action = i->second;
}

void
Sprite::update(float /*delta*/)
{
  //time += 10*delta;
  //std::cout << "Delta: " << delta << std::endl;
}

void
Sprite::draw(DrawingContext& context, const Vector& pos, int layer,
    Uint32 drawing_effect)
{
  time = SDL_GetTicks();
  unsigned int frame = get_current_frame();

  if (frame < action->surfaces.size())
  {
    Surface* surface = action->surfaces[frame];
    
    context.draw_surface(surface, pos - Vector(action->x_hotspot, action->y_hotspot), layer, drawing_effect);
  }
}

#if 0
void
Sprite::draw_part(float sx, float sy, float x, float y, float w, float h)
{
  time = SDL_GetTicks();
  unsigned int frame = get_current_frame();

  if (frame < surfaces.size())
    surfaces[frame]->draw_part(sx, sy, x - x_hotspot, y - y_hotspot, w, h);
}
#endif

void
Sprite::reset()
{
  time = 0;
}

int
Sprite::get_current_frame() const
{
  unsigned int frame = static_cast<int>(fmodf(time, action->surfaces.size()*action->frame_delay)/action->frame_delay);
  return frame % action->surfaces.size();
}

int
Sprite::get_width() const
{
  return action->surfaces[get_current_frame()]->w;
}

int
Sprite::get_height() const
{
  return action->surfaces[get_current_frame()]->h;
}

/* EOF */
