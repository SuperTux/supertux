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
int next_level;
int score_multiplier;
timer_type  super_bkgd_timer;

// FIXME: Move this into a view class
float scroll_x;

unsigned int global_frame_counter;

timer_type time_left;
double frame_ratio;

// EOF //

