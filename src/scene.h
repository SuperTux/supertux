//
// C Interface: scene
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef SUPERTUX_SCENE_H
#define SUPERTUX_SCENE_H

#include "texture.h"
#include "timer.h"

#define FRAME_RATE 10 // 100 Frames per second (10ms)

// Player stats
struct PlayerStatus
{
  int  score;
  int  distros;
  int  next_level;
  int  score_multiplier;
};

extern PlayerStatus player_status;

extern float scroll_x;
extern unsigned int global_frame_counter;

extern timer_type time_left;

#endif /*SUPERTUX_SCENE_H*/
