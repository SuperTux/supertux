
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// by Ricardo Cruz <rick2@aeiou.pt>

#ifndef SUPERTUX_MOUSECURSOR_H
#define SUPERTUX_MOUSECURSOR_H

#include <string>
#include "timer.h"
#include "texture.h"

#define MC_FRAME_PERIOD 800  // in ms

#define MC_STATES_NB 3
enum {
  MC_NORMAL,
  MC_CLICK,
  MC_LINK
};

class MouseCursor
  {
    public:
    MouseCursor(std::string cursor_file, int frames);
    ~MouseCursor();
    int state();
    void set_state(int nstate);
    void draw();
    
    private:
    int state_before_click;
    int cur_state;
    int cur_frame, tot_frames;
    texture_type cursor;
    Timer timer;
  };

#endif /*SUPERTUX_MOUSECURSOR_H*/
