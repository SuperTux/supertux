//
// C Implementation: type
//
// Description:
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <SDL/SDL_image.h>
#include "setup.h"
#include "globals.h"
#include "screen.h"
#include "defines.h"
#include "type.h"
#include "scene.h"

double get_frame_ratio(base_type* pbase)
{
  unsigned int cur_time = SDL_GetTicks();
  double frame_ratio = (float)(cur_time-pbase->updated)/(float)(FRAME_RATE);
  pbase->updated = cur_time;
  return (frame_ratio == 0 ? 1 : frame_ratio);
}
