//
// C Implementation: collision
//
// Description:
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "defines.h"
#include "collision.h"
#include "bitmask.h"
#include "scene.h"

int rectcollision(base_type* one, base_type* two)
{

  if (one->x >= two->x - one->width &&
      one->x <= two->x + two->width  &&
      one->y >= two->y - one->height &&
      one->y <= two->y + two->height )
    {
      return YES;
    }
  else
    {
      return NO;
    }
}

int rectcollision_offset(base_type* one, base_type* two, float off_x, float off_y)
{

  if (one->x >= two->x - one->width +off_x &&
      one->x <= two->x + two->width + off_x &&
      one->y >= two->y - one->height + off_y &&
      one->y <= two->y + two->height + off_y )
    {
      return YES;
    }
  else
    {
      return NO;
    }
}

void collision_handler()
{
  int i,j;

  /* CO_BULLET & CO_BADGUY check */
  for(i = 0; i < num_bullets; ++i)
    {
      if(bullets[i].base.alive)
        {
          for(j = 0; j < num_bad_guys; ++j)
            {
              if(bad_guys[j].dying == NO && bad_guys[j].base.alive)
                {
                  if(rectcollision(&bullets[i].base,&bad_guys[j].base) == YES)
                    {
                      /* We have detected a collision and now call the collision functions of the collided objects. */
                      bullet_collision(&bullets[i], CO_BADGUY);
                      badguy_collision(&bad_guys[j], &bullets[i], CO_BULLET);
                    }
                }
            }
        }
    }

  /* CO_BADGUY & CO_BADGUY check */
  for(i = 0; i < num_bad_guys; ++i)
    {
      if(bad_guys[i].base.alive)
        {
          for(j = i+1; j < num_bad_guys; ++j)
            {
              if(j != i && bad_guys[j].base.alive)
                {
                  if(rectcollision(&bad_guys[i].base,&bad_guys[j].base) == YES)
                    {
                      /* We have detected a collision and now call the collision functions of the collided objects. */
                      badguy_collision(&bad_guys[j], &bad_guys[i], CO_BADGUY);
                      badguy_collision(&bad_guys[i], &bad_guys[j], CO_BADGUY);
                    }
                }
            }
        }
    }

  /* CO_BADGUY & CO_PLAYER check */
  for(i = 0; i < num_bad_guys; ++i)
    {
      if(bad_guys[i].base.alive)
        {
          if(bad_guys[i].dying == NO && rectcollision_offset(&bad_guys[i].base,&tux.base,0,0) == YES )
            {
              /* We have detected a collision and now call the collision functions of the collided objects. */
              if (tux.base.ym > 0 && bad_guys[i].kind != BAD_MONEY)
                {
                  badguy_collision(&bad_guys[i], &tux, CO_PLAYER);
                }
              else
                {
                  player_collision(&tux, &bad_guys[i], CO_BADGUY);
                }
            }
        }
    }

  /* CO_UPGRADE & CO_PLAYER check */
  for(i = 0; i < num_upgrades; ++i)
    {
      if(upgrades[i].base.alive)
        {
          if(rectcollision(&upgrades[i].base,&tux.base) == YES)
            {
              /* We have detected a collision and now call the collision functions of the collided objects. */
              upgrade_collision(&upgrades[i], &tux, CO_PLAYER);
            }

        }
    }

}


