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

bool rectcollision(base_type* one, base_type* two)
{
  return (one->x >= two->x - one->width + 1  &&
          one->x <= two->x + two->width - 1  &&
          one->y >= two->y - one->height + 1 &&
          one->y <= two->y + two->height - 1);
}

bool rectcollision_offset(base_type* one, base_type* two, float off_x, float off_y)
{
  return (one->x >= two->x - one->width +off_x + 1 &&
          one->x <= two->x + two->width + off_x - 1 &&
          one->y >= two->y - one->height + off_y + 1 &&
          one->y <= two->y + two->height + off_y - 1);
}

bool collision_object_map(base_type* pbase)
{
  int v = (int)pbase->height / 16;
  int h = (int)pbase->width / 16;

  if(issolid(pbase->x + 1, pbase->y + 1) ||
      issolid(pbase->x + pbase->width -1, pbase->y + 1) ||
      issolid(pbase->x +1, pbase->y + pbase->height -1) ||
      issolid(pbase->x + pbase->width -1, pbase->y + pbase->height - 1))
    return true;

  for(int i = 1; i < h; ++i)
    {
      if(issolid(pbase->x + i*16,pbase->y + 1))
        return true;
    }

  for(int i = 1; i < h; ++i)
    {
      if(  issolid(pbase->x + i*16,pbase->y + pbase->height - 1))
        return true;
    }

  for(int i = 1; i < v; ++i)
    {
      if(  issolid(pbase->x + 1, pbase->y + i*16))
        return true;
    }
  for(int i = 1; i < v; ++i)
    {
      if(  issolid(pbase->x + pbase->width - 1, pbase->y + i*16))
        return true;
    }

  return false;
}


void collision_swept_object_map(base_type* old, base_type* current)
{
  int steps; /* Used to speed up the collision tests, by stepping every 16pixels in the path. */
  int h;
  float lpath; /* Holds the longest path, which is either in X or Y direction. */
  float xd,yd; /* Hold the smallest steps in X and Y directions. */
  float temp, xt, yt; /* Temporary variable. */

  lpath = 0;
  xd = 0;
  yd = 0;

  if(old->x == current->x && old->y == current->y)
    {
      return;
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

  for(float i = 0; i <= lpath; old->x += xd, old->y += yd, ++i)
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
  // CO_BULLET & CO_BADGUY check
  for(unsigned int i = 0; i < bullets.size(); ++i)
    {
      for(unsigned int j = 0; j < bad_guys.size(); ++j)
        {
          if(bad_guys[j].dying != DYING_NOT)
            continue;
          if(rectcollision(&bullets[i].base, &bad_guys[j].base))
            {
              // We have detected a collision and now call the
              // collision functions of the collided objects.
              // collide with bad_guy first, since bullet_collision will
              // delete the bullet
              bad_guys[j].collision(0, CO_BULLET);
              bullet_collision(&bullets[i], CO_BADGUY);
              break; // bullet is invalid now, so break
            }
        }
    }

  /* CO_BADGUY & CO_BADGUY check */
  for(unsigned int i = 0; i < bad_guys.size(); ++i)
    {
      if(bad_guys[i].dying != DYING_NOT)
        continue;
      
      for(unsigned int j = i+1; j < bad_guys.size(); ++j)
        {
          if(j == i || bad_guys[j].dying != DYING_NOT)
            continue;

          if(rectcollision(&bad_guys[i].base, &bad_guys[j].base))
            {
              // We have detected a collision and now call the
              // collision functions of the collided objects.
              bad_guys[j].collision(&bad_guys[i], CO_BADGUY);
              bad_guys[i].collision(&bad_guys[j], CO_BADGUY);
            }
        }
    }

  if(tux.dying != DYING_NOT) return;
    
  // CO_BADGUY & CO_PLAYER check 
  for(unsigned int i = 0; i < bad_guys.size(); ++i)
    {
      if(bad_guys[i].dying != DYING_NOT)
        continue;
      
      if(rectcollision_offset(&bad_guys[i].base,&tux.base,0,0))
        {
          // We have detected a collision and now call the collision
          // functions of the collided objects.
          if (tux.previous_base.y < tux.base.y &&
              tux.previous_base.y + tux.previous_base.height < bad_guys[i].base.y + bad_guys[i].base.height/2)
            {
              bad_guys[i].collision(&tux, CO_PLAYER, COLLISION_SQUISH);
            }
          else
            {
              tux.collision(&bad_guys[i], CO_BADGUY);
            }
        }
    }

  // CO_UPGRADE & CO_PLAYER check
  for(unsigned int i = 0; i < upgrades.size(); ++i)
    {
      if(rectcollision(&upgrades[i].base,&tux.base))
        {
          // We have detected a collision and now call the collision
          // functions of the collided objects.
          upgrade_collision(&upgrades[i], &tux, CO_PLAYER);
        }
    }

}


