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

void bouncy_distro_action(bouncy_distro_type* pbouncy_distro)
{
      if (pbouncy_distro->alive)
        {
          pbouncy_distro->y = pbouncy_distro->y + pbouncy_distro->ym;

          pbouncy_distro->ym++;

          if (pbouncy_distro->ym >= 0)
            pbouncy_distro->alive = NO;
        }
}

void bouncy_distro_draw(bouncy_distro_type* pbouncy_distro)
{
      if (pbouncy_distro->alive)
        {
          texture_draw(&img_distro[0],
                    pbouncy_distro->x - scroll_x,
                    pbouncy_distro->y,
                    NO_UPDATE);
        }
}

void broken_brick_action(broken_brick_type* pbroken_brick)
{
      if (pbroken_brick->alive)
        {
          pbroken_brick->x = pbroken_brick->x + pbroken_brick->xm;
          pbroken_brick->y = pbroken_brick->y + pbroken_brick->ym;

          pbroken_brick->ym++;

          if (pbroken_brick->ym >= 0)
            pbroken_brick->alive = NO;
        }
}

void broken_brick_draw(broken_brick_type* pbroken_brick)
{
      if (pbroken_brick->alive)
        {
          src.x = rand() % 16;
          src.y = rand() % 16;
          src.w = 16;
          src.h = 16;

          dest.x = pbroken_brick->x - scroll_x;
          dest.y = pbroken_brick->y;
          dest.w = 16;
          dest.h = 16;

          SDL_BlitSurface(img_brick[0].sdl_surface, &src, screen, &dest);
        }
}

void bouncy_brick_action(bouncy_brick_type* pbouncy_brick)
{
      if (pbouncy_brick->alive)
        {
          pbouncy_brick->offset = (pbouncy_brick->offset +
                                     pbouncy_brick->offset_m);

          /* Go back down? */

          if (pbouncy_brick->offset < -BOUNCY_BRICK_MAX_OFFSET)
            pbouncy_brick->offset_m = BOUNCY_BRICK_SPEED;


          /* Stop bouncing? */

          if (pbouncy_brick->offset == 0)
            pbouncy_brick->alive = NO;
        }
}

void bouncy_brick_draw(bouncy_brick_type* pbouncy_brick)
{
      if (pbouncy_brick->alive)
        {
          if (pbouncy_brick->x >= scroll_x - 32 &&
              pbouncy_brick->x <= scroll_x + screen->w)
            {
              dest.x = pbouncy_brick->x - scroll_x;
              dest.y = pbouncy_brick->y;
              dest.w = 32;
              dest.h = 32;

              SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format,
                                                     current_level.bkgd_red,
                                                     current_level.bkgd_green,
                                                     current_level.bkgd_blue));

              drawshape(pbouncy_brick->x - scroll_x,
                        pbouncy_brick->y + pbouncy_brick->offset,
                        pbouncy_brick->shape);
            }
        }
}

void floating_score_init(floating_score_type* pfloating_score, int x, int y, int s)
{
      pfloating_score->alive = YES;
      pfloating_score->x = x;
      pfloating_score->y = y - 16;
      timer_start(&pfloating_score->timer,1000);
      pfloating_score->value = s;
}

void floating_score_action(floating_score_type* pfloating_score)
{
      if (pfloating_score->alive)
        {
          pfloating_score->y = pfloating_score->y - 2;

      if(!timer_check(&pfloating_score->timer))
          pfloating_score->alive = NO;
        }
}

void floating_score_draw(floating_score_type* pfloating_score)
{
      if (pfloating_score->alive)
        {
	char str[10];
          sprintf(str, "%d", pfloating_score->value);
          drawtext(str,
                   pfloating_score->x + 16 - strlen(str) * 8,
                   pfloating_score->y,
                   letters_gold, NO_UPDATE, 1);
        }
}

