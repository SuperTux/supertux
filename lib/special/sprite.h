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

#include "../utils/lispreader.h"
#include "../video/surface.h"
#include "../math/vector.h"

namespace SuperTux
  {

  class Sprite
    {
    public:
      /** cur has to be a pointer to data in the form of ((x-hotspot 5)
          (y-hotspot 10) ...) */
      Sprite(lisp_object_t* cur);
      ~Sprite();

      void reset();

      /** Update the sprite and process to the next frame */
      void update(float delta);
      void draw(DrawingContext& context, const Vector& pos, int layer,
                Uint32 drawing_effect = NONE_EFFECT);
      int get_current_frame() const;

      float get_fps()
      {
        return fps;
      } ;
      int get_frames()
      {
        return surfaces.size();
      } ;

      std::string get_name() const
        {
          return name;
        }
      int get_width() const;
      int get_height() const;

      Surface* get_frame(unsigned int frame)
      {
        if(frame < surfaces.size())
          return surfaces[frame];
        else
          return surfaces[0];
      }    
    private:
      std::string name;

      int x_hotspot;
      int y_hotspot;

      /** Frames per second */
      float fps;

      /** Number of seconds that a frame is displayed until it is switched
          to the next frame */
      float frame_delay;

      float time;

      std::vector<Surface*> surfaces;

      void init_defaults();
    };

} //namespace SuperTux

#endif /*SUPERTUX_SPRITE_H*/

/* Local Variables: */
/* mode:c++ */
/* End: */
