//
// C Implementation: world
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
#include <string.h>
#include "globals.h"
#include "scene.h"
#include "screen.h"
#include "defines.h"
#include "world.h"

texture_type img_distro[4];

void bouncy_distro_init(bouncy_distro_type* pbouncy_distro, float x, float y)
{
  pbouncy_distro->base.alive = YES;
  pbouncy_distro->base.x = x;
  pbouncy_distro->base.y = y;
  pbouncy_distro->base.ym = -2;
}

void bouncy_distro_action(bouncy_distro_type* pbouncy_distro)
{
  if (pbouncy_distro->base.alive)
    {
      pbouncy_distro->base.y = pbouncy_distro->base.y + pbouncy_distro->base.ym * frame_ratio;

      pbouncy_distro->base.ym += 0.1 * frame_ratio;

      if (pbouncy_distro->base.ym >= 0)
        pbouncy_distro->base.alive = NO;
    }
}

void bouncy_distro_draw(bouncy_distro_type* pbouncy_distro)
{
  if (pbouncy_distro->base.alive)
    {
      texture_draw(&img_distro[0],
                   pbouncy_distro->base.x - scroll_x,
                   pbouncy_distro->base.y,
                   NO_UPDATE);
    }
}

void broken_brick_init(broken_brick_type* pbroken_brick, float x, float y, float xm, float ym)
{
  pbroken_brick->base.alive = YES;
  pbroken_brick->base.x = x;
  pbroken_brick->base.y = y;
  pbroken_brick->base.xm = xm;
  pbroken_brick->base.ym = ym;
  timer_init(&pbroken_brick->timer,YES);
  timer_start(&pbroken_brick->timer,200);
}

void broken_brick_action(broken_brick_type* pbroken_brick)
{
  if (pbroken_brick->base.alive)
    {
      pbroken_brick->base.x = pbroken_brick->base.x + pbroken_brick->base.xm * frame_ratio;
      pbroken_brick->base.y = pbroken_brick->base.y + pbroken_brick->base.ym * frame_ratio;

      if (!timer_check(&pbroken_brick->timer))
        pbroken_brick->base.alive = NO;
    }
}

void broken_brick_draw(broken_brick_type* pbroken_brick)
{
SDL_Rect src, dest;
  if (pbroken_brick->base.alive)
    {
      src.x = rand() % 16;
      src.y = rand() % 16;
      src.w = 16;
      src.h = 16;

      dest.x = (int)(pbroken_brick->base.x - scroll_x);
      dest.y = (int)pbroken_brick->base.y;
      dest.w = 16;
      dest.h = 16;

      texture_draw_part(&img_brick[0],src.x,src.y,dest.x,dest.y,dest.w,dest.h,NO_UPDATE);
    }
}

void bouncy_brick_init(bouncy_brick_type* pbouncy_brick, float x, float y)
{
  pbouncy_brick->base.alive = YES;
  pbouncy_brick->base.x = x;
  pbouncy_brick->base.y = y;
  pbouncy_brick->offset = 0;
  pbouncy_brick->offset_m = -BOUNCY_BRICK_SPEED;
  pbouncy_brick->shape = shape(x, y);
}

void bouncy_brick_action(bouncy_brick_type* pbouncy_brick)
{

  if (pbouncy_brick->base.alive)
    {

      pbouncy_brick->offset = (pbouncy_brick->offset +
                               pbouncy_brick->offset_m * frame_ratio);

      /* Go back down? */

      if (pbouncy_brick->offset < -BOUNCY_BRICK_MAX_OFFSET)
        pbouncy_brick->offset_m = BOUNCY_BRICK_SPEED;


      /* Stop bouncing? */

      if (pbouncy_brick->offset >= 0)
        pbouncy_brick->base.alive = NO;
    }
}

void bouncy_brick_draw(bouncy_brick_type* pbouncy_brick)
{
  int s;
  SDL_Rect dest;
  
  if (pbouncy_brick->base.alive)
    {
      if (pbouncy_brick->base.x >= scroll_x - 32 &&
          pbouncy_brick->base.x <= scroll_x + screen->w)
        {
          dest.x = (int)(pbouncy_brick->base.x - scroll_x);
          dest.y = (int)pbouncy_brick->base.y;
          dest.w = 32;
          dest.h = 32;

          if(current_level.bkgd_image[0] == '\0')
            {
              fillrect(pbouncy_brick->base.x - scroll_x,pbouncy_brick->base.y,32,32,current_level.bkgd_red,current_level.bkgd_green,
                       current_level.bkgd_blue,0);
            }
          else
            {
              s = (int)scroll_x / 30;
              texture_draw_part(&img_bkgd,dest.x + s,dest.y,dest.x,dest.y,dest.w,dest.h,NO_UPDATE);
            }

          drawshape(pbouncy_brick->base.x - scroll_x,
                    pbouncy_brick->base.y + pbouncy_brick->offset,
                    pbouncy_brick->shape);
        }
    }
}

void floating_score_init(floating_score_type* pfloating_score, float x, float y, int s)
{
  pfloating_score->base.alive = YES;
  pfloating_score->base.x = x;
  pfloating_score->base.y = y - 16;
  timer_init(&pfloating_score->timer,YES);
  timer_start(&pfloating_score->timer,1000);
  pfloating_score->value = s;
}

void floating_score_action(floating_score_type* pfloating_score)
{
  if (pfloating_score->base.alive)
    {
      pfloating_score->base.y = pfloating_score->base.y - 2 * frame_ratio;

      if(!timer_check(&pfloating_score->timer))
        pfloating_score->base.alive = NO;
    }
}

void floating_score_draw(floating_score_type* pfloating_score)
{
  if (pfloating_score->base.alive)
    {
      char str[10];
      sprintf(str, "%d", pfloating_score->value);
      text_draw(&gold_text, str, (int)pfloating_score->base.x + 16 - strlen(str) * 8, (int)pfloating_score->base.y, 1, NO_UPDATE);
    }
}

