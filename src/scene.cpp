//
// C Implementation: scene
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <stdlib.h>
#include "scene.h"

int score;
int distros;
int level;
int next_level;
int score_multiplier;
bool counting_distros;
int distro_counter;
timer_type  super_bkgd_timer;

// FIXME: Move this into a view class
float scroll_x;

unsigned int global_frame_counter;

texture_type img_box_full;
texture_type img_box_empty;
texture_type img_mints;
texture_type img_coffee;
texture_type img_super_bkgd;
texture_type img_red_glow;
timer_type time_left;
double frame_ratio;

// EOF //

