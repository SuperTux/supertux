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

  if (one->x >= two->x - one->width + 1 &&
      one->x <= two->x + two->width - 1  &&
      one->y >= two->y - one->height + 1&&
      one->y <= two->y + two->height - 1)
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
  if (one->x >= two->x - one->width +off_x + 1 &&
      one->x <= two->x + two->width + off_x - 1 &&
      one->y >= two->y - one->height + off_y + 1 &&
      one->y <= two->y + two->height + off_y - 1)
    {
      return YES;
    }
  else
    {
      return NO;
    }
}

int collision_object_map(base_type* pbase)
{
  int v,h,i;

  v = (int)pbase->height / 16;
  h = (int)pbase->width / 16;

  if(issolid(pbase->x + 1, pbase->y + 1) ||
      issolid(pbase->x + pbase->width -1, pbase->y + 1) ||
      issolid(pbase->x +1, pbase->y + pbase->height -1) ||
      issolid(pbase->x + pbase->width -1, pbase->y + pbase->height - 1))
    return YES;

  for(i = 1; i < h; ++i)
    {
      if(issolid(pbase->x + i*16,pbase->y + 1))
        return YES;
    }

  for(i = 1; i < h; ++i)
    {
      if(  issolid(pbase->x + i*16,pbase->y + pbase->height - 1))
        return YES;
    }

  for(i = 1; i < v; ++i)
    {
      if(  issolid(pbase->x + 1, pbase->y + i*16))
        return YES;
    }
  for(i = 1; i < v; ++i)
    {
      if(  issolid(pbase->x + pbase->width - 1, pbase->y + i*16))
        return YES;
    }

  return NO;
}


int collision_swept_object_map(base_type* old, base_type* current)
{
  int steps; /* Used to speed up the collision tests, by stepping every 16pixels in the path. */
  int h;
  float i;
  float lpath; /* Holds the longest path, which is either in X or Y direction. */
  float xd,yd; /* Hold the smallest steps in X and Y directions. */
  float temp, xt, yt; /* Temporary variable. */

  lpath = 0;
  xd = 0;
  yd = 0;

  if(old->x == current->x && old->y == current->y)
    {
      return 0;
    }
  else if(old->x == current->x && old->y != current->y)
    {
      lpath = current->y - old->y;
      if(lpath < 0)
        {
          yd = -1;
          lpath = -lpath;
        }
      else
        {
          yd = 1;
        }

      h = 1;
      xd = 0;

    }
  else if(old->x != current->x && old->y == current->y)
    {
      lpath = current->x - old->x;
      if(lpath < 0)
        {
          xd = -1;
          lpath = -lpath;
        }
      else
        {
          xd = 1;
        }
      h = 2;
      yd = 0;
    }
  else
    {
      lpath = current->x - old->x;
      if(lpath < 0)
        lpath = -lpath;
      if(current->y - old->y > lpath || old->y - current->y > lpath)
        lpath = current->y - old->y;
      if(lpath < 0)
        lpath = -lpath;
      h = 3;
      xd = (current->x - old->x) / lpath;
      yd = (current->y - old->y) / lpath;
    }

  steps = (int)(lpath / (float)16);

  old->x += xd;
  old->y += yd;

  for(i = 0; i <= lpath; old->x += xd, old->y += yd, ++i)
    {
      if(steps > 0)
        {
          old->y += yd*16.;
          old->x += xd*16.;
          steps--;
        }

      if(collision_object_map(old))
        {
          switch(h)
            {
            case 1:
              current->y = old->y - yd;
              while(collision_object_map(current))
                current->y -= yd;
              break;
            case 2:
              current->x = old->x - xd;
              while(collision_object_map(current))
                current->x -= xd;
              break;
            case 3:
              xt = current->x;
              yt = current->y;
              current->x = old->x - xd;
              current->y = old->y - yd;
              while(collision_object_map(current))
                {
                  current->x -= xd;
                  current->y -= yd;
                }

              temp = current->x;
              current->x = xt;
              if(!collision_object_map(current))
                break;
              current->x = temp;
              temp = current->y;
              current->y = yt;

              if(!collision_object_map(current))
                {
                  break;
                }
              else
                {
                  current->y = temp;
                  while(!collision_object_map(current))
                    current->y += yd;
		  current->y -= yd;
                  break;
                }

              break;
            default:
              break;
            }
          break;
        }
    }

  *old = *current;

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
      if(bad_guys[i].base.alive && bad_guys[i].dying == NO)
        {
          for(j = i+1; j < num_bad_guys; ++j)
            {
              if(j != i && bad_guys[j].base.alive && bad_guys[j].dying == NO)
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
              if (tux.previous_base.y < tux.base.y &&
                  tux.previous_base.y + tux.previous_base.height < bad_guys[i].base.y + bad_guys[i].base.height/2 &&
                  bad_guys[i].kind != BAD_MONEY && bad_guys[i].mode != HELD)
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


