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

PlayerStatus player_status;

PlayerStatus::PlayerStatus()
  : score(0),
    distros(0),
    lives(3)
{
}

// FIXME: Move this into a view class
float scroll_x;

unsigned int global_frame_counter;

// EOF //

