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

int rectcollision(itop_type* one, itop_type* two)
{

  if (*one->x >= *two->x - *one->width &&
      *one->x <= *two->x + *two->width  &&
      *one->y >= *two->y - *one->height &&
      *one->y <= *two->y + *two->height )
    {
      return YES;
    }
  else
    {
      return NO;
    }
}

int rectcollision_offset(itop_type* one, itop_type* two, float off_x, float off_y)
{

  if (*one->x >= *two->x - *one->width +off_x &&
      *one->x <= *two->x + *two->width + off_x &&
      *one->y >= *two->y - *one->height + off_y &&
      *one->y <= *two->y + *two->height + off_y )
    {
      return YES;
    }
  else
    {
      return NO;
    }
}

void collision_rect_detect(int co_one, int co_two)
{
  int i,j;

  /* CO_BULLET & CO_BADGUY check */
  for(i = 0; i < NUM_BULLETS; ++i)
    {
      if(bullets[i].alive)
        {
          for(j = 0; j < NUM_BAD_GUYS; ++j)
            {
              if(bad_guys[j].alive)
                {
                  if(rectcollision(&bullets[i].it,&bad_guys[j].it) == YES)
                    {
		    /* We have detected a collision and now call the collision functions of the collided objects. */
		      bullet_collision(&bullets[i], CO_BADGUY);
		      badguy_collision(&bad_guys[j], &bullets[i], CO_BULLET);
                    }
                }
            }
        }
    }
}

void collision_handler()
{
  int i,j;

  /* CO_BULLET & CO_BADGUY check */
  for(i = 0; i < NUM_BULLETS; ++i)
    {
      if(bullets[i].alive)
        {
          for(j = 0; j < NUM_BAD_GUYS; ++j)
            {
              if(bad_guys[j].alive)
                {
                  if(rectcollision(&bullets[i].it,&bad_guys[j].it) == YES)
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
  for(i = 0; i < NUM_BAD_GUYS; ++i)
    {
      if(bad_guys[i].alive)
        {
          for(j = i+1; j < NUM_BAD_GUYS; ++j)
            {
              if(j != i && bad_guys[j].alive)
                {
                  if(rectcollision(&bad_guys[i].it,&bad_guys[j].it) == YES)
                    {
		    /* We have detected a collision and now call the collision functions of the collided objects. */
		      badguy_collision(&bad_guys[j], &bad_guys[i], CO_BADGUY);
                    }
                }
            }
        }
    }

    /* CO_BADGUY & CO_PLAYER check */
  for(i = 0; i < NUM_BAD_GUYS; ++i)
    {
      if(bad_guys[i].alive)
        {
		  if(rectcollision_offset(&bad_guys[i].it,&tux.it,0,48) == YES && tux.ym < 0)
                    {
		    /* We have detected a collision and now call the collision functions of the collided objects. */
		      badguy_collision(&bad_guys[i], &tux, CO_PLAYER);
		      }
		   if(rectcollision(&bad_guys[i].it,&tux.it) == YES)
		   {
		      player_collision(&tux, &bad_guys[i], CO_BADGUY);
		   }

        }
    }

    /* CO_UPGRADE & CO_PLAYER check */
  for(i = 0; i < NUM_UPGRADES; ++i)
    {
      if(upgrades[i].alive)
        {
		  if(rectcollision(&upgrades[i].it,&tux.it) == YES)
                    {
		    /* We have detected a collision and now call the collision functions of the collided objects. */
		      upgrade_collision(&upgrades[i], &tux, CO_PLAYER);
		      }

        }
    }
    
}


