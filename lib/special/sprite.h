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

#ifndef SUPERTUX_SPRITE_H
#define SUPERTUX_SPRITE_H

#include <string>
#include <vector>
#include <map>

#include "../utils/lispreader.h"
#include "../video/surface.h"
#include "../math/vector.h"

namespace SuperTux
  {

  class Sprite
    {
    private:

      struct Action
        {
        std::string name;

        int x_offset;
        int y_offset;

        /** Frames per second */
        float fps;

        std::vector<Surface*> surfaces;
        };

    public:
      /** cur has to be a pointer to data in the form of ((x-offset 5)
          (y-offset 10) ...) */
      Sprite(lisp_object_t* cur);
      ~Sprite();

      /** Draw sprite, automatically calculates next frame */
      void draw(DrawingContext& context, const Vector& pos, int layer,
                Uint32 drawing_effect = NONE_EFFECT);

      /** Set action (or state) */
      void set_action(std::string act);

      /* Start an animation
          -1 - for infinite
          0  - stopped
          1,2,3  - one, two, three times... */
      void start_animation(int loops);
      /* Stop animation */
      void stop_animation()
        { start_animation(0); }
      /** Check if animation is stopped or not */
      bool check_animation();
      /** Reverse the animation */
      void reverse_animation(bool reverse);

      float get_fps()
        { return action->fps; }
      /** Get current action total frames */
      int get_frames()
        { return action->surfaces.size(); }
      /** Get sprite's name */
      std::string get_name() const
        { return name; }
      /** Get current action name */
      std::string get_action_name() const
        { return action->name; }
      int get_width();
      int get_height();

      /** Get current frame */
      int get_frame()
        { return (int)frame; }
      /** Set current frame */
      void set_frame(int frame_)
        { if(frame_ > get_frames()) frame = 0; else frame = frame_; }
      Surface* get_frame(unsigned int frame)
      {
        if(frame < action->surfaces.size())
          return action->surfaces[frame];
        else
          return action->surfaces[0];
      }    
    private:
      void init_defaults(Action* act);
      void parse_action(LispReader& lispreader);

      void update();
      void reset();

      std::string name;

      float frame;
      int animation_loops;
      bool animation_reversed;
      float last_tick;

      typedef std::map <std::string, Action*> Actions;
      Actions actions;

      Action* action;
     std::string next_action;
    };

} //namespace SuperTux

#endif /*SUPERTUX_SPRITE_H*/

/* Local Variables: */
/* mode:c++ */
/* End: */
