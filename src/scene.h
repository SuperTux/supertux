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

extern int  score;
extern int  distros;
extern int  level; 
extern int  next_level;
extern int  score_multiplier;
extern bool counting_distros;
extern int  distro_counter;

extern timer_type  super_bkgd_timer;
extern float scroll_x;
extern unsigned int global_frame_counter;

extern texture_type img_box_full, img_box_empty, img_mints, img_coffee, img_super_bkgd, img_red_glow;
extern timer_type time_left;
extern double frame_ratio;

#endif /*SUPERTUX_SCENE_H*/
