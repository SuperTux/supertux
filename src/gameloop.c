/*
  gameloop.c
  
  Super Tux - Game Loop!
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - December 9, 2003
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <SDL.h>

#ifdef LINUX
#include <pwd.h>
#include <sys/types.h>
#include <ctype.h>
#endif

#include "defines.h"
#include "globals.h"
#include "gameloop.h"
#include "screen.h"
#include "sound.h"
#include "setup.h"
#include "high_scores.h"
#include "menu.h"


/* Sound files: */

enum {
  SND_JUMP,
  SND_BIGJUMP,
  SND_SKID,
  SND_DISTRO,
  SND_HERRING,
  SND_BRICK,
  SND_HURT,
  SND_SQUISH,
  SND_FALL,
  SND_RICOCHET,
  SND_BUMP_UPGRADE,
  SND_UPGRADE,
  SND_EXCELLENT,
  SND_COFFEE,
  SND_SHOOT,
  SND_LIFEUP
};


char * soundfilenames[NUM_SOUNDS] = {
                                      DATA_PREFIX "/sounds/jump.wav",
                                      DATA_PREFIX "/sounds/bigjump.wav",
                                      DATA_PREFIX "/sounds/skid.wav",
                                      DATA_PREFIX "/sounds/distro.wav",
                                      DATA_PREFIX "/sounds/herring.wav",
                                      DATA_PREFIX "/sounds/brick.wav",
                                      DATA_PREFIX "/sounds/hurt.wav",
                                      DATA_PREFIX "/sounds/squish.wav",
                                      DATA_PREFIX "/sounds/fall.wav",
                                      DATA_PREFIX "/sounds/ricochet.wav",
                                      DATA_PREFIX "/sounds/bump-upgrade.wav",
                                      DATA_PREFIX "/sounds/upgrade.wav",
                                      DATA_PREFIX "/sounds/excellent.wav",
                                      DATA_PREFIX "/sounds/coffee.wav",
                                      DATA_PREFIX "/sounds/shoot.wav",
                                      DATA_PREFIX "/sounds/lifeup.wav"
                                    };


/* Local variables: */

int score, highscore, distros, level, lives, scroll_x, next_level, game_pause,
done, quit, tux_dir, tux_size, tux_duck, tux_x, tux_xm, tux_y, tux_ym,
tux_dying, tux_safe, jumping, jump_counter, frame, score_multiplier,
tux_frame_main, tux_frame, tux_got_coffee, tux_skidding,
super_bkgd_time, time_left, tux_invincible_time, endpos,
counting_distros, distro_counter;
int bkgd_red, bkgd_green, bkgd_blue, level_width;
int left, right, up, down, fire, old_fire;
SDL_Surface * img_brick[2], * img_solid[4], * img_distro[4],
* img_waves[3], * img_water, * img_pole, * img_poletop, * img_flag[2];
SDL_Surface * img_bkgd[2][4];
SDL_Surface * img_golden_herring;
SDL_Surface * img_bsod_left[4], * img_bsod_right[4],
* img_laptop_left[3], * img_laptop_right[3],
* img_money_left[2], * img_money_right[2];
SDL_Surface * img_bsod_squished_left, * img_bsod_squished_right,
* img_bsod_falling_left, * img_bsod_falling_right,
* img_laptop_flat_left, * img_laptop_flat_right,
* img_laptop_falling_left, * img_laptop_falling_right;
SDL_Surface * img_box_full, * img_box_empty, * img_mints, * img_coffee,
* img_super_bkgd, * img_bullet, * img_red_glow;
SDL_Surface * img_cloud[2][4];
SDL_Surface * tux_right[3], * tux_left[3],
* bigtux_right[3], * bigtux_left[3],
* bigtux_right_jump, * bigtux_left_jump,
* cape_right[2], * cape_left[2],
* bigcape_right[2], * bigcape_left[2],
* ducktux_right, * ducktux_left,
* skidtux_right, * skidtux_left;
SDL_Event event;
SDL_Rect src, dest;
SDLKey key;
unsigned char * tiles[15];
bouncy_distro_type bouncy_distros[NUM_BOUNCY_DISTROS];
broken_brick_type broken_bricks[NUM_BROKEN_BRICKS];
bouncy_brick_type bouncy_bricks[NUM_BOUNCY_BRICKS];
bad_guy_type bad_guys[NUM_BAD_GUYS];
floating_score_type floating_scores[NUM_FLOATING_SCORES];
upgrade_type upgrades[NUM_UPGRADES];
bullet_type bullets[NUM_BULLETS];
char song_title[20];
char levelname[20];
char leveltheme[20];
char str[10];

/* Local function prototypes: */

void initgame(void);
void loadlevel(void);
void loadlevelgfx(void);
void loadlevelsong(void);
void unloadlevelgfx(void);
void unloadlevelsong(void);
void loadshared(void);
void unloadshared(void);
void drawshape(int x, int y, unsigned char c);
unsigned char shape(int x, int y, int sx);
int issolid(int x, int y, int sx);
int isbrick(int x, int y, int sx);
int isice(int x, int y, int sx);
int isfullbox(int x, int y, int sx);
void change(int x, int y, int sx, unsigned char c);
void trybreakbrick(int x, int y, int sx);
void bumpbrick(int x, int y, int sx);
void tryemptybox(int x, int y, int sx);
void trygrabdistro(int x, int y, int sx, int bounciness);
void add_bouncy_distro(int x, int y);
void add_broken_brick(int x, int y);
void add_broken_brick_piece(int x, int y, int xm, int ym);
void add_bouncy_brick(int x, int y);
void add_bad_guy(int x, int y, int kind);
void add_score(int x, int y, int s);
void trybumpbadguy(int x, int y, int sx);
void add_upgrade(int x, int y, int kind);
void killtux(int mode);
void add_bullet(int x, int y, int dir, int xm);
void drawendscreen(void);
void drawresultscreen(void);

/* --- GAME EVENT! --- */

void game_event(void)
{

  while (SDL_PollEvent(&event))
    {

      if (event.type == SDL_QUIT)
        {
          /* Quit event - quit: */

          quit = 1;
        }
      else if (event.type == SDL_KEYDOWN)
        {
          /* A keypress! */

          key = event.key.keysym.sym;

          if (key == SDLK_ESCAPE)
            {
              /* Escape: Open/Close the menu: */
              if(!game_pause)
                {
                  if(show_menu)
                    show_menu = 0;
                  else
                    show_menu = 1;
                }
            }
          else if (key == SDLK_RIGHT)
            {
              right = DOWN;
            }
          else if (key == SDLK_LEFT)
            {
              left = DOWN;
            }
          else if (key == SDLK_UP)
            {
              up = DOWN;
            }
          else if (key == SDLK_DOWN)
            {
              down = DOWN;
            }
          else if (key == SDLK_LCTRL)
            {
              fire = DOWN;
            }
        }
      else if (event.type == SDL_KEYUP)
        {
          /* A keyrelease! */

          key = event.key.keysym.sym;

          /* Check for menu-events, if the menu is shown */
          if(show_menu)
            menu_event(key);

          if (key == SDLK_RIGHT)
            {
              right = UP;
            }
          else if (key == SDLK_LEFT)
            {
              left = UP;
            }
          else if (key == SDLK_UP)
            {
              up = UP;
            }
          else if (key == SDLK_DOWN)
            {
              down = UP;
            }
          else if (key == SDLK_LCTRL)
            {
              fire = UP;
            }
          else if (key == SDLK_p)
            {
              if(!show_menu)
                {
                  if(game_pause)
                    game_pause = 0;
                  else
                    game_pause = 1;
                }
            }
          else if (key == SDLK_TAB)
            {
              tux_size = !tux_size;
            }
          else if (key == SDLK_END)
            {
              distros += 50;
            }
          else if (key == SDLK_SPACE)
            {
              next_level = 1;
            }
        }
#ifdef JOY_YES
      else if (event.type == SDL_JOYAXISMOTION)
        {
          if (event.jaxis.axis == JOY_X)
            {
              if (event.jaxis.value < -256)
                left = DOWN;
              else
                left = UP;

              if (event.jaxis.value > 256)
                right = DOWN;
              else
                right = UP;
            }
          else if (event.jaxis.axis == JOY_Y)
            {
              if (event.jaxis.value > 256)
                down = DOWN;
              else
                down = UP;

              /* Handle joystick for the menu */
              if(show_menu)
                {
                  if(down == DOWN)
                    menuaction = MN_DOWN;
                  else
                    menuaction = MN_UP;
                }
            }
        }
      else if (event.type == SDL_JOYBUTTONDOWN)
        {
          if (event.jbutton.button == JOY_A)
            up = DOWN;
          else if (event.jbutton.button == JOY_B)
            fire = DOWN;
        }
      else if (event.type == SDL_JOYBUTTONUP)
        {
          if (event.jbutton.button == JOY_A)
            up = UP;
          else if (event.jbutton.button == JOY_B)
            fire = UP;

          if(show_menu)
            menuaction = MN_HIT;

        }
#endif

    }

}

/* --- GAME ACTION! --- */

int game_action(void)
{
  int i,j;

  /* --- HANDLE TUX! --- */

  /* Handle key and joystick state: */

  if (!(tux_dying || next_level))
    {
      if (right == DOWN && left == UP)
        {
          if (jumping == NO)
            {
              if (tux_xm < -SKID_XM && !tux_skidding &&
                  tux_dir == LEFT)
                {
                  tux_skidding = SKID_TIME;

                  play_sound(sounds[SND_SKID]);

                }
              tux_dir = RIGHT;
            }

          if (tux_xm < 0 && !isice(tux_x, tux_y + 32, scroll_x) &&
              !tux_skidding)
            {
              tux_xm = 0;
            }

          if (!tux_duck)
            {
              if (tux_dir == RIGHT)
                {
                  /* Facing the direction we're jumping?  Go full-speed: */

                  if (fire == UP)
                    {
                      tux_xm = tux_xm + WALK_SPEED;

                      if (tux_xm > MAX_WALK_XM)
                        tux_xm = MAX_WALK_XM;
                    }
                  else if (fire == DOWN)
                    {
                      tux_xm = tux_xm + RUN_SPEED;

                      if (tux_xm > MAX_RUN_XM)
                        tux_xm = MAX_RUN_XM;
                    }
                }
              else
                {
                  /* Not facing the direction we're jumping?
                  Go half-speed: */

                  tux_xm = tux_xm + WALK_SPEED / 2;

                  if (tux_xm > MAX_WALK_XM / 2)
                    tux_xm = MAX_WALK_XM / 2;
                }
            }
        }
      else if (left == DOWN && right == UP)
        {
          if (jumping == NO)
            {
              if (tux_xm > SKID_XM && !tux_skidding &&
                  tux_dir == RIGHT)
                {
                  tux_skidding = SKID_TIME;
                  play_sound(sounds[SND_SKID]);
                }
              tux_dir = LEFT;
            }

          if (tux_xm > 0 && !isice(tux_x, tux_y + 32, scroll_x) &&
              !tux_skidding)
            {
              tux_xm = 0;
            }

          if (!tux_duck)
            {
              if (tux_dir == LEFT)
                {
                  /* Facing the direction we're jumping?  Go full-speed: */

                  if (fire == UP)
                    {
                      tux_xm = tux_xm - WALK_SPEED;

                      if (tux_xm < -MAX_WALK_XM)
                        tux_xm = -MAX_WALK_XM;
                    }
                  else if (fire == DOWN)
                    {
                      tux_xm = tux_xm - RUN_SPEED;

                      if (tux_xm < -MAX_RUN_XM)
                        tux_xm = -MAX_RUN_XM;
                    }
                }
              else
                {
                  /* Not facing the direction we're jumping?
                  Go half-speed: */

                  tux_xm = tux_xm - WALK_SPEED / 2;

                  if (tux_xm < -MAX_WALK_XM / 2)
                    tux_xm = -MAX_WALK_XM / 2;
                }
            }
        }


      /* End of level? */

      if (tux_x >= endpos && endpos != 0)
        {
          next_level = 1;
        }


      /* Jump/jumping? */

      if (up == DOWN)
        {
          if (jump_counter == 0)
            {
              /* Taking off? */

              if (!issolid(tux_x, tux_y + 32, scroll_x) ||
                  tux_ym != 0)
                {
                  /* If they're not on the ground, or are currently moving
                  vertically, don't jump! */

                  jump_counter = MAX_JUMP_COUNT;
                }
              else
                {
                  /* Make sure we're not standing back up into a solid! */

                  if (tux_size == SMALL || tux_duck == NO ||
                      !issolid(tux_x, tux_y, scroll_x))
                    {
                      jumping = YES;

                      if (tux_size == SMALL)
                        play_sound(sounds[SND_JUMP]);
                      else
                        play_sound(sounds[SND_BIGJUMP]);
                    }
                }
            }


          /* Keep jumping for a while: */

          if (jump_counter < MAX_JUMP_COUNT)
            {
              tux_ym = tux_ym - JUMP_SPEED;
              jump_counter++;
            }
        }
      else
        jump_counter = 0;


      /* Shoot! */

      if (fire == DOWN && old_fire == UP && tux_got_coffee)
        {
          add_bullet(tux_x + scroll_x, tux_y, tux_dir, tux_xm);
        }


      /* Duck! */

      if (down == DOWN)
        {
          if (tux_size == BIG)
            tux_duck = YES;
        }
      else
        {
          if (tux_size == BIG && tux_duck == YES)
            {
              /* Make sure we're not standing back up into a solid! */

              if (!issolid(tux_x, tux_y - 32, scroll_x))
                tux_duck = NO;
            }
          else
            tux_duck = NO;
        }
    } /* (tux_dying || next_level) */
  else
    {
      /* Tux either died, or reached the end of a level! */


      if (playing_music())
        halt_music();


      if (next_level)
        {
          /* End of a level! */
          level++;
          next_level = 0;
          drawresultscreen();
        }
      else
        {

          tux_ym = tux_ym + GRAVITY;



          /* He died :^( */

          lives--;

          /* No more lives!? */

          if (lives < 0)
            {
              drawendscreen();

              if (score > highscore)
                save_hs(score);

	      unloadlevelgfx();
              unloadlevelsong();
              unloadshared();	      
              return(0);
            } /* if (lives < 0) */
        }

      /* Either way, (re-)load the (next) level... */

      loadlevel();
      unloadlevelgfx();
      loadlevelgfx();
      unloadlevelsong();
      loadlevelsong();
    }

  /* Move tux: */

  tux_x = tux_x + tux_xm;
  tux_y = tux_y + tux_ym;


  /* Keep tux in bounds: */

  if (tux_x < 0)
    tux_x = 0;
  else if (tux_x > 320 && scroll_x < ((level_width * 32) - 640))
    {
      /* Scroll the screen in past center: */

      scroll_x = scroll_x + (tux_x - 320);
      tux_x = 320;

      if (scroll_x > ((level_width * 32) - 640))
        scroll_x = ((level_width * 32) - 640);
    }
  else if (tux_x > 608)
    {
      /* ... unless there's no more to scroll! */

      tux_x = 608;
    }


  /* Land: */

  if (!tux_dying)
    {
      if (issolid(tux_x, tux_y + 31, scroll_x) &&
          !issolid(tux_x - tux_xm, tux_y + 31, scroll_x))
        {
          while (issolid(tux_x, tux_y + 31, scroll_x))
            {
              if (tux_xm < 0)
                tux_x++;
              else if (tux_xm > 0)
                tux_x--;
            }

          tux_xm = 0;
        }

      if (issolid(tux_x, tux_y, scroll_x) &&
          !issolid(tux_x - tux_xm, tux_y, scroll_x))
        {
          while (issolid(tux_x, tux_y, scroll_x))
            {
              if (tux_xm < 0)
                tux_x++;
              else if (tux_xm > 0)
                tux_x--;
            }

          tux_xm = 0;
        }

      if (issolid(tux_x, tux_y + 31, scroll_x))
        {
          /* Set down properly: */

          while (issolid(tux_x, tux_y + 31, scroll_x))
            {
              if (tux_ym < 0)
                tux_y++;
              else if (tux_ym > 0)
                tux_y--;
            }


          /* Reset score multiplier (for mutli-hits): */

          if (tux_ym > 0)
            score_multiplier = 1;


          /* Stop jumping! */

          tux_ym = 0;
          jumping = NO;
        }


      /* Bump into things: */

      if (issolid(tux_x, tux_y, scroll_x) ||
          (tux_size == BIG && !tux_duck &&
           (issolid(tux_x, tux_y - 32, scroll_x))))
        {
          if (!issolid(tux_x - tux_xm, tux_y, scroll_x) &&
              (tux_size == SMALL || tux_duck ||
               !issolid(tux_x - tux_xm, tux_y - 32, scroll_x)))
            {
              tux_x = tux_x - tux_xm;
              tux_xm = 0;
            }
          else if (!issolid(tux_x, tux_y - tux_ym, scroll_x) &&
                   (tux_size == SMALL || tux_duck ||
                    !issolid(tux_x, tux_y - 32 - tux_ym, scroll_x)))
            {
              if (tux_ym <= 0)
                {
                  /* Jumping up? */

                  if (tux_size == BIG)
                    {
                      /* Break bricks and empty boxes: */

                      if (!tux_duck)
                        {
                          if (isbrick(tux_x, tux_y - 32, scroll_x) ||
                              isfullbox(tux_x, tux_y - 32, scroll_x))
                            {
                              trygrabdistro(tux_x, tux_y - 64, scroll_x,
                                            BOUNCE);
                              trybumpbadguy(tux_x, tux_y - 96, scroll_x);

                              if (isfullbox(tux_x, tux_y - 32,
                                            scroll_x))
                                {
                                  bumpbrick(tux_x, tux_y - 32,
                                            scroll_x);
                                }

                              trybreakbrick(tux_x, tux_y - 32, scroll_x);
                              tryemptybox(tux_x, tux_y - 32, scroll_x);
                            }

                          if (isbrick(tux_x + 31, tux_y - 32, scroll_x) ||
                              isfullbox(tux_x + 31, tux_y - 32, scroll_x))
                            {
                              trygrabdistro(tux_x + 31,
                                            tux_y - 64,
                                            scroll_x,
                                            BOUNCE);
                              trybumpbadguy(tux_x + 31,
                                            tux_y - 96,
                                            scroll_x);

                              if (isfullbox(tux_x + 31, tux_y - 32,
                                            scroll_x))
                                {
                                  bumpbrick(tux_x + 31, tux_y - 32,
                                            scroll_x);
                                }

                              trybreakbrick(tux_x + 31,
                                            tux_y - 32,
                                            scroll_x);
                              tryemptybox(tux_x + 31,
                                          tux_y - 32,
                                          scroll_x);
                            }
                        }
                      else /* ducking */
                        {
                          if (isbrick(tux_x, tux_y, scroll_x) ||
                              isfullbox(tux_x, tux_y, scroll_x))
                            {
                              trygrabdistro(tux_x, tux_y - 32, scroll_x,
                                            BOUNCE);
                              trybumpbadguy(tux_x, tux_y - 64, scroll_x);
                              if (isfullbox(tux_x, tux_y, scroll_x))
                                bumpbrick(tux_x, tux_y, scroll_x);
                              trybreakbrick(tux_x, tux_y, scroll_x);
                              tryemptybox(tux_x, tux_y, scroll_x);
                            }

                          if (isbrick(tux_x + 31, tux_y, scroll_x) ||
                              isfullbox(tux_x + 31, tux_y, scroll_x))
                            {
                              trygrabdistro(tux_x + 31,
                                            tux_y - 32,
                                            scroll_x,
                                            BOUNCE);
                              trybumpbadguy(tux_x + 31,
                                            tux_y - 64,
                                            scroll_x);
                              if (isfullbox(tux_x + 31, tux_y, scroll_x))
                                bumpbrick(tux_x + 31, tux_y, scroll_x);
                              trybreakbrick(tux_x + 31, tux_y, scroll_x);
                              tryemptybox(tux_x + 31, tux_y, scroll_x);
                            }
                        }
                    }
                  else
                    {
                      /* It's a brick and we're small, make the brick
                         bounce, and grab any distros above it: */

                      if (isbrick(tux_x, tux_y, scroll_x) ||
                          isfullbox(tux_x, tux_y, scroll_x))
                        {
                          trygrabdistro(tux_x, tux_y - 32, scroll_x,
                                        BOUNCE);
                          trybumpbadguy(tux_x, tux_y - 64, scroll_x);
                          bumpbrick(tux_x, tux_y, scroll_x);
                          tryemptybox(tux_x, tux_y, scroll_x);
                        }

                      if (isbrick(tux_x + 31, tux_y, scroll_x) ||
                          isfullbox(tux_x + 31, tux_y, scroll_x))
                        {
                          trygrabdistro(tux_x + 31, tux_y - 32, scroll_x,
                                        BOUNCE);
                          trybumpbadguy(tux_x + 31, tux_y - 64, scroll_x);
                          bumpbrick(tux_x + 31, tux_y, scroll_x);
                          tryemptybox(tux_x + 31, tux_y, scroll_x);
                        }


                      /* Get a distro from a brick? */

                      if (shape(tux_x, tux_y, scroll_x) == 'x' ||
                          shape(tux_x, tux_y, scroll_x) == 'y')
                        {
                          add_bouncy_distro(((tux_x + scroll_x + 1)
                                             / 32) * 32,
                                            (tux_y / 32) * 32);

                          if (counting_distros == NO)
                            {
                              counting_distros = YES;
                              distro_counter = 100;
                            }

                          if (distro_counter <= 0)
                            change(tux_x, tux_y, scroll_x, 'a');

                          play_sound(sounds[SND_DISTRO]);
                          score = score + SCORE_DISTRO;
                          distros++;
                        }
                      else if (shape(tux_x + 31, tux_y, scroll_x) == 'x' ||
                               shape(tux_x + 31, tux_y, scroll_x) == 'y')
                        {
                          add_bouncy_distro(((tux_x + scroll_x + 1 + 31)
                                             / 32) * 32,
                                            (tux_y / 32) * 32);

                          if (counting_distros == NO)
                            {
                              counting_distros = YES;
                              distro_counter = 100;
                            }

                          if (distro_counter <= 0)
                            change(tux_x + 31, tux_y, scroll_x, 'a');

                          play_sound(sounds[SND_DISTRO]);
                          score = score + SCORE_DISTRO;
                          distros++;
                        }
                    }


                  /* Bump head: */

                  tux_y = (tux_y / 32) * 32 + 30;
                }
              else
                {
                  /* Land on feet: */

                  tux_y = (tux_y / 32) * 32 - 32;
                }

              tux_ym = 0;
              jumping = NO;
              jump_counter = MAX_JUMP_COUNT;
            }
        }
    }


  /* Grab distros: */

  if (!tux_dying)
    {
      trygrabdistro(tux_x, tux_y, scroll_x, NO_BOUNCE);
      trygrabdistro(tux_x + 31, tux_y, scroll_x, NO_BOUNCE);

      if (tux_size == BIG && !tux_duck)
        {
          trygrabdistro(tux_x, tux_y - 32, scroll_x, NO_BOUNCE);
          trygrabdistro(tux_x + 31, tux_y - 32, scroll_x, NO_BOUNCE);
        }
    }


  /* Enough distros for a One-up? */

  if (distros >= DISTROS_LIFEUP)
    {
      distros = distros - DISTROS_LIFEUP;
      if(lives < MAX_LIVES)
        lives++;
      play_sound(sounds[SND_LIFEUP]); /*We want to hear the sound even, if MAX_LIVES is reached*/
    }


  /* Keep in-bounds, vertically: */

  if (tux_y < 0)
    tux_y = 0;
  else if (tux_y > 480)
    {
      killtux(KILL);
    }


  /* Slow down horizontally: */

  if (!tux_dying)
    {
      if (right == UP && left == UP)
        {
          if (isice(tux_x, tux_y + 32, scroll_x) ||
              !issolid(tux_x, tux_y + 32, scroll_x))
            {
              /* Slowly on ice or in air: */

              if (tux_xm > 0)
                tux_xm--;
              else if (tux_xm < 0)
                tux_xm++;
            }
          else
            {
              /* Quickly, otherwise: */

              tux_xm = tux_xm / 2;
            }
        }


      /* Drop vertically: */

      if (!issolid(tux_x, tux_y + 32, scroll_x))
        {
          tux_ym = tux_ym + GRAVITY;

          if (tux_ym > MAX_YM)
            tux_ym = MAX_YM;
        }
    }


  if (tux_safe > 0)
    tux_safe--;


  /* ---- DONE HANDLING TUX! --- */


  /* Handle bouncy distros: */

  for (i = 0; i < NUM_BOUNCY_DISTROS; i++)
    {
      if (bouncy_distros[i].alive)
        {
          bouncy_distros[i].y = bouncy_distros[i].y + bouncy_distros[i].ym;

          bouncy_distros[i].ym++;

          if (bouncy_distros[i].ym >= 0)
            bouncy_distros[i].alive = NO;
        }
    }


  /* Handle broken bricks: */

  for (i = 0; i < NUM_BROKEN_BRICKS; i++)
    {
      if (broken_bricks[i].alive)
        {
          broken_bricks[i].x = broken_bricks[i].x + broken_bricks[i].xm;
          broken_bricks[i].y = broken_bricks[i].y + broken_bricks[i].ym;

          broken_bricks[i].ym++;

          if (broken_bricks[i].ym >= 0)
            broken_bricks[i].alive = NO;
        }
    }


  /* Handle distro counting: */

  if (counting_distros == YES)
    {
      distro_counter--;

      if (distro_counter <= 0)
        counting_distros = -1;
    }


  /* Handle bouncy bricks: */

  for (i = 0; i < NUM_BOUNCY_BRICKS; i++)
    {
      if (bouncy_bricks[i].alive)
        {
          bouncy_bricks[i].offset = (bouncy_bricks[i].offset +
                                     bouncy_bricks[i].offset_m);

          /* Go back down? */

          if (bouncy_bricks[i].offset < -BOUNCY_BRICK_MAX_OFFSET)
            bouncy_bricks[i].offset_m = BOUNCY_BRICK_SPEED;


          /* Stop bouncing? */

          if (bouncy_bricks[i].offset == 0)
            bouncy_bricks[i].alive = NO;
        }
    }


  /* Handle floating scores: */

  for (i = 0; i < NUM_FLOATING_SCORES; i++)
    {
      if (floating_scores[i].alive)
        {
          floating_scores[i].y = floating_scores[i].y - 2;
          floating_scores[i].timer--;

          if (floating_scores[i].timer <= 0)
            floating_scores[i].alive = NO;
        }
    }


  /* Handle bullets: */

  for (i = 0; i < NUM_BULLETS; i++)
    {
      if (bullets[i].alive)
        {
          bullets[i].x = bullets[i].x + bullets[i].xm;
          bullets[i].y = bullets[i].y + bullets[i].ym;

          if (issolid(bullets[i].x, bullets[i].y, 0))
            {
              if (issolid(bullets[i].x, bullets[i].y - bullets[i].ym, 0))
                bullets[i].alive = NO;
              else
                {
                  if (bullets[i].ym >= 0)
                    {
                      bullets[i].y = (bullets[i].y / 32) * 32 - 8;
                    }
                  bullets[i].ym = -bullets[i].ym;
                }
            }

          bullets[i].ym = bullets[i].ym + GRAVITY;

          if (bullets[i].x < scroll_x ||
              bullets[i].x > scroll_x + 640)
            {
              bullets[i].alive = NO;
            }
        }


      if (bullets[i].alive)
        {
          for (j = 0; j < NUM_BAD_GUYS; j++)
            {
              if (bad_guys[j].alive && !bad_guys[j].dying)
                {
                  if (bullets[i].x >= bad_guys[j].x - 4 &&
                      bullets[i].x <= bad_guys[j].x + 32 + 4 &&
                      bullets[i].y >= bad_guys[j].y - 4 &&
                      bullets[i].y <= bad_guys[j].y + 32 + 4)
                    {
                      /* Kill the bad guy! */

                      bullets[i].alive = 0;
                      bad_guys[j].dying = FALLING;
                      bad_guys[j].ym = -8;


                      /* Gain some points: */

                      if (bad_guys[j].kind == BAD_BSOD)
                        {
                          add_score(bad_guys[j].x - scroll_x, bad_guys[j].y,
                                    50 * score_multiplier);
                        }
                      else if (bad_guys[j].kind == BAD_LAPTOP)
                        {
                          add_score(bad_guys[j].x - scroll_x, bad_guys[j].y,
                                    25 * score_multiplier);
                        }


                      /* Play death sound: */
                      play_sound(sounds[SND_FALL]);
                    }
                }
            }
        }
    }


  /* Handle background timer: */

  if (super_bkgd_time)
    super_bkgd_time--;


  /* Handle invincibility timer: */


  if (tux_invincible_time > 50)
    {
      tux_invincible_time--;


      if (!playing_music())
        play_music( herring_song, 1 );
    }
  else
    {
      if (current_music == HERRING_MUSIC)
        {
          /* stop the herring_song, now play the level_song ! */
          current_music = LEVEL_MUSIC;
          halt_music();
        }
      if (!playing_music())
        play_music( level_song, 1 );
      if (tux_invincible_time > 0)
        tux_invincible_time--;
    }


  /* Handle upgrades: */

  for (i = 0; i < NUM_UPGRADES; i++)
    {
      if (upgrades[i].alive)
        {
          if (upgrades[i].height < 32)
            {
              /* Rise up! */

              upgrades[i].height++;
            }
          else
            {
              /* Move around? */

              if (upgrades[i].kind == UPGRADE_MINTS ||
                  upgrades[i].kind == UPGRADE_HERRING)
                {
                  upgrades[i].x = upgrades[i].x + upgrades[i].xm;
                  upgrades[i].y = upgrades[i].y + upgrades[i].ym;

                  if (issolid(upgrades[i].x, upgrades[i].y + 31, 0) ||
                      issolid(upgrades[i].x + 31, upgrades[i].y + 31, 0))
                    {
                      if (upgrades[i].ym > 0)
                        {
                          if (upgrades[i].kind == UPGRADE_MINTS)
                            {
                              upgrades[i].ym = 0;
                            }
                          else if (upgrades[i].kind == UPGRADE_HERRING)
                            {
                              upgrades[i].ym = -24;
                            }

                          upgrades[i].y = (upgrades[i].y / 32) * 32;
                        }
                    }
                  else
                    upgrades[i].ym = upgrades[i].ym + GRAVITY;

                  if (issolid(upgrades[i].x, upgrades[i].y, 0))
                    {
                      upgrades[i].xm = -upgrades[i].xm;
                    }
                }


              /* Off the screen?  Kill it! */

              if (upgrades[i].x < scroll_x)
                upgrades[i].alive = NO;


              /* Did the player grab it? */

              if (tux_x + scroll_x >= upgrades[i].x - 32 &&
                  tux_x + scroll_x <= upgrades[i].x + 32 &&
                  tux_y >= upgrades[i].y - 32 &&
                  tux_y <= upgrades[i].y + 32)
                {
                  /* Remove the upgrade: */

                  upgrades[i].alive = NO;


                  /* Affect the player: */

                  if (upgrades[i].kind == UPGRADE_MINTS)
                    {
                      play_sound(sounds[SND_EXCELLENT]);
                      tux_size = BIG;
                      super_bkgd_time = 8;
                    }
                  else if (upgrades[i].kind == UPGRADE_COFFEE)
                    {
                      play_sound(sounds[SND_COFFEE]);
                      tux_got_coffee = YES;
                      super_bkgd_time = 4;
                    }
                  else if (upgrades[i].kind == UPGRADE_HERRING)
                    {
                      play_sound(sounds[SND_HERRING]);
                      tux_invincible_time = TUX_INVINCIBLE_TIME;
                      super_bkgd_time = 4;
                      /* play the herring song ^^ */
                      current_music = HERRING_MUSIC;
                      if (playing_music())
                        halt_music();
                      play_music( herring_song, 1 );
                    }
                }
            }
        }
    }


  /* Handle bad guys: */

  for (i = 0; i < NUM_BAD_GUYS; i++)
    {
      if (bad_guys[i].alive)
        {
          if (bad_guys[i].seen)
            {
              if (bad_guys[i].kind == BAD_BSOD)
                {
                  /* --- BLUE SCREEN OF DEATH MONSTER: --- */

                  /* Move left/right: */

                  if (bad_guys[i].dying == NO ||
                      bad_guys[i].dying == FALLING)
                    {
                      if (bad_guys[i].dir == RIGHT)
                        bad_guys[i].x = bad_guys[i].x + 4;
                      else if (bad_guys[i].dir == LEFT)
                        bad_guys[i].x = bad_guys[i].x - 4;
                    }


                  /* Move vertically: */

                  bad_guys[i].y = bad_guys[i].y + bad_guys[i].ym;


                  /* Bump into things horizontally: */

                  if (!bad_guys[i].dying)
                    {
                      if (issolid(bad_guys[i].x, bad_guys[i].y, 0))
                        bad_guys[i].dir = !bad_guys[i].dir;
                    }


                  /* Bump into other bad guys: */

                  for (j = 0; j < NUM_BAD_GUYS; j++)
                    {
                      if (j != i && bad_guys[j].alive &&
                          !bad_guys[j].dying && !bad_guys[i].dying &&
                          bad_guys[i].x >= bad_guys[j].x - 32 &&
                          bad_guys[i].x <= bad_guys[j].x + 32 &&
                          bad_guys[i].y >= bad_guys[j].y - 32 &&
                          bad_guys[i].y <= bad_guys[j].y + 32)
                        {
                          bad_guys[i].dir = !bad_guys[i].dir;
                        }
                    }


                  /* Fall if we get off the ground: */

                  if (bad_guys[i].dying != FALLING)
                    {
                      if (!issolid(bad_guys[i].x, bad_guys[i].y + 32, 0) &&
                          bad_guys[i].ym < MAX_YM)
                        {
                          bad_guys[i].ym = bad_guys[i].ym + GRAVITY;
                        }
                      else
                        {
                          /* Land: */

                          if (bad_guys[i].ym > 0)
                            {
                              bad_guys[i].y = (bad_guys[i].y / 32) * 32;
                              bad_guys[i].ym = 0;
                            }
                        }
                    }
                  else
                    bad_guys[i].ym = bad_guys[i].ym + GRAVITY;

                  if (bad_guys[i].y > 480)
                    bad_guys[i].alive = NO;
                }
              else if (bad_guys[i].kind == BAD_LAPTOP)
                {
                  /* --- LAPTOP MONSTER: --- */

                  /* Move left/right: */

                  if (bad_guys[i].mode != FLAT && bad_guys[i].mode != KICK)
                    {
                      if (bad_guys[i].dying == NO ||
                          bad_guys[i].dying == FALLING)
                        {
                          if (bad_guys[i].dir == RIGHT)
                            bad_guys[i].x = bad_guys[i].x + 4;
                          else if (bad_guys[i].dir == LEFT)
                            bad_guys[i].x = bad_guys[i].x - 4;
                        }
                    }
                  else if (bad_guys[i].mode == KICK)
                    {
                      if (bad_guys[i].dir == RIGHT)
                        bad_guys[i].x = bad_guys[i].x + 16;
                      else if (bad_guys[i].dir == LEFT)
                        bad_guys[i].x = bad_guys[i].x - 16;
                    }


                  /* Move vertically: */

                  bad_guys[i].y = bad_guys[i].y + bad_guys[i].ym;


                  /* Bump into things horizontally: */

                  if (!bad_guys[i].dying)
                    {
                      if (issolid(bad_guys[i].x, bad_guys[i].y, 0))
                        {
                          bad_guys[i].dir = !bad_guys[i].dir;

                          if (bad_guys[i].mode == KICK)
                            play_sound(sounds[SND_RICOCHET]);
                        }
                    }


                  /* Bump into other bad guys: */

                  for (j = 0; j < NUM_BAD_GUYS; j++)
                    {
                      if (j != i && bad_guys[j].alive &&
                          !bad_guys[j].dying && !bad_guys[i].dying &&
                          bad_guys[i].x >= bad_guys[j].x - 32 &&
                          bad_guys[i].x <= bad_guys[j].x + 32 &&
                          bad_guys[i].y >= bad_guys[j].y - 32 &&
                          bad_guys[i].y <= bad_guys[j].y + 32)
                        {
                          if (bad_guys[i].mode != KICK)
                            bad_guys[i].dir = !bad_guys[i].dir;
                          else
                            {
                              /* We're in kick mode, kill the other guy: */

                              bad_guys[j].dying = FALLING;
                              bad_guys[j].ym = -8;
                              play_sound(sounds[SND_FALL]);

                              add_score(bad_guys[i].x - scroll_x,
                                        bad_guys[i].y, 100);
                            }
                        }
                    }


                  /* Fall if we get off the ground: */

                  if (bad_guys[i].dying != FALLING)
                    {
                      if (!issolid(bad_guys[i].x, bad_guys[i].y + 32, 0) &&
                          bad_guys[i].ym < MAX_YM)
                        {
                          bad_guys[i].ym = bad_guys[i].ym + GRAVITY;
                        }
                      else
                        {
                          /* Land: */

                          if (bad_guys[i].ym > 0)
                            {
                              bad_guys[i].y = (bad_guys[i].y / 32) * 32;
                              bad_guys[i].ym = 0;
                            }
                        }
                    }
                  else
                    bad_guys[i].ym = bad_guys[i].ym + GRAVITY;

                  if (bad_guys[i].y > 480)
                    bad_guys[i].alive = NO;
                }
              else if (bad_guys[i].kind == BAD_MONEY)
                {
                  /* --- MONEY BAGS: --- */


                  /* Move vertically: */

                  bad_guys[i].y = bad_guys[i].y + bad_guys[i].ym;


                  /* Fall if we get off the ground: */

                  if (bad_guys[i].dying != FALLING)
                    {
                      if (!issolid(bad_guys[i].x, bad_guys[i].y + 32, 0))
                        {
                          if (bad_guys[i].ym < MAX_YM)
                            {
                              bad_guys[i].ym = bad_guys[i].ym + GRAVITY;
                            }
                        }
                      else
                        {
                          /* Land: */

                          if (bad_guys[i].ym > 0)
                            {
                              bad_guys[i].y = (bad_guys[i].y / 32) * 32;
                              bad_guys[i].ym = -MAX_YM;
                            }
                        }
                    }
                  else
                    bad_guys[i].ym = bad_guys[i].ym + GRAVITY;

                  if (bad_guys[i].y > 480)
                    bad_guys[i].alive = NO;
                }
              else if (bad_guys[i].kind == -1)
              {}


              /* Kill it if the player jumped on it: */

              if (!bad_guys[i].dying && !tux_dying && !tux_safe &&
                  tux_x + scroll_x >= bad_guys[i].x - 32 &&
                  tux_x + scroll_x <= bad_guys[i].x + 32 &&
                  tux_y >= bad_guys[i].y - 32 &&
                  tux_y <= bad_guys[i].y - 8
                  /* &&
                  tux_ym >= 0 */)
                {
                  if (bad_guys[i].kind == BAD_BSOD)
                    {
                      bad_guys[i].dying = SQUISHED;
                      bad_guys[i].timer = 16;
                      tux_ym = -KILL_BOUNCE_YM;

                      add_score(bad_guys[i].x - scroll_x, bad_guys[i].y,
                                50 * score_multiplier);

                      play_sound(sounds[SND_SQUISH]);
                    }
                  else if (bad_guys[i].kind == BAD_LAPTOP)
                    {
                      if (bad_guys[i].mode != FLAT)
                        {
                          /* Flatten! */

                          bad_guys[i].mode = FLAT;

                          bad_guys[i].timer = 64;

                          tux_y = tux_y - 32;
                        }
                      else
                        {
                          /* Kick! */

                          bad_guys[i].mode = KICK;

                          if (tux_x + scroll_x <= bad_guys[i].x)
                            bad_guys[i].dir = RIGHT;
                          else
                            bad_guys[i].dir = LEFT;

                          bad_guys[i].timer = 8;
                        }

                      tux_ym = -KILL_BOUNCE_YM;

                      add_score(bad_guys[i].x - scroll_x,
                                bad_guys[i].y,
                                25 * score_multiplier);

                      /* play_sound(sounds[SND_SQUISH]); */
                    }
                  else if (bad_guys[i].kind == -1)
                  {}

                  score_multiplier++;
                }


              /* Hurt the player if he just touched it: */

              if (!bad_guys[i].dying && !tux_dying &&
                  !tux_safe &&
                  tux_x + scroll_x >= bad_guys[i].x - 32 &&
                  tux_x + scroll_x <= bad_guys[i].x + 32 &&
                  tux_y >= bad_guys[i].y - 32 &&
                  tux_y <= bad_guys[i].y + 32)
                {
                  if (bad_guys[i].mode == FLAT)
                    {
                      /* Kick: */

                      bad_guys[i].mode = KICK;

                      if (tux_x + scroll_x <= bad_guys[i].x)
                        {
                          bad_guys[i].dir = RIGHT;
                          bad_guys[i].x = bad_guys[i].x + 16;
                        }
                      else
                        {
                          bad_guys[i].dir = LEFT;
                          bad_guys[i].x = bad_guys[i].x - 16;
                        }

                      bad_guys[i].timer = 8;
                    }
                  else if (bad_guys[i].mode == KICK)
                    {
                      if (tux_y < bad_guys[i].y - 16 &&
                          bad_guys[i].timer == 0)
                        {
                          /* Step on (stop being kicked) */

                          bad_guys[i].mode = FLAT;
                          bad_guys[i].timer = 64;
                        }
                      else
                        {
                          /* Hurt if you get hit by kicked laptop: */

                          if (bad_guys[i].timer == 0)
                            {
                              if (tux_invincible_time == 0)
                                {
                                  killtux(SHRINK);
                                }
                              else
                                {
                                  bad_guys[i].dying = FALLING;
                                  bad_guys[i].ym = -8;
                                  play_sound(sounds[SND_FALL]);
                                }
                            }
                        }
                    }
                  else
                    {
                      if (tux_invincible_time == 0)
                        {
                          killtux(SHRINK);
                        }
                      else
                        {
                          bad_guys[i].dying = FALLING;
                          bad_guys[i].ym = -8;
                          play_sound(sounds[SND_FALL]);
                        }
                    }
                }


              /* Handle mode timer: */

              if (bad_guys[i].mode == FLAT)
                {
                  bad_guys[i].timer--;

                  if (bad_guys[i].timer <= 0)
                    bad_guys[i].mode = NORMAL;
                }
              else if (bad_guys[i].mode == KICK)
                {
                  if (bad_guys[i].timer > 0)
                    bad_guys[i].timer--;
                }


              /* Handle dying timer: */

              if (bad_guys[i].dying == SQUISHED)
                {
                  bad_guys[i].timer--;


                  /* Remove it if time's up: */

                  if (bad_guys[i].timer <= 0)
                    bad_guys[i].alive = NO;
                }


              /* Remove if it's far off the screen: */

              if (bad_guys[i].x < scroll_x - OFFSCREEN_DISTANCE)
                bad_guys[i].alive = NO;
            }
          else /* !seen */
            {
              /* Once it's on screen, it's activated! */

              if (bad_guys[i].x <= scroll_x + 640 + OFFSCREEN_DISTANCE)
                bad_guys[i].seen = YES;
            }
        }
    }


  /* Handle skidding: */

  if (tux_skidding > 0)
    {
      tux_skidding--;
    }

  return -1;
}

/* --- GAME DRAW! --- */

void game_draw()
{
  int  x, y, i;

  /* Draw screen: */

  if (tux_dying && (frame % 4) == 0)
    clearscreen(255, 255, 255);
  else
    {
      if (super_bkgd_time == 0)
        clearscreen(bkgd_red, bkgd_green, bkgd_blue);
      else
        drawimage(img_super_bkgd, 0, 0, NO_UPDATE);
    }


  /* Draw background: */

  for (y = 0; y < 15; y++)
    {
      for (x = 0; x < 21; x++)
        {
          drawshape(x * 32 - (scroll_x % 32), y * 32,
                    tiles[y][x + (scroll_x / 32)]);
        }
    }


  /* (Bouncy bricks): */

  for (i = 0; i < NUM_BOUNCY_BRICKS; i++)
    {
      if (bouncy_bricks[i].alive)
        {
          if (bouncy_bricks[i].x >= scroll_x - 32 &&
              bouncy_bricks[i].x <= scroll_x + 640)
            {
              dest.x = bouncy_bricks[i].x - scroll_x;
              dest.y = bouncy_bricks[i].y;
              dest.w = 32;
              dest.h = 32;

              SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format,
                                                     bkgd_red,
                                                     bkgd_green,
                                                     bkgd_blue));

              drawshape(bouncy_bricks[i].x - scroll_x,
                        bouncy_bricks[i].y + bouncy_bricks[i].offset,
                        bouncy_bricks[i].shape);
            }
        }
    }


  /* (Bad guys): */

  for (i = 0; i < NUM_BAD_GUYS; i++)
    {
      if (bad_guys[i].alive &&
          bad_guys[i].x > scroll_x - 32 &&
          bad_guys[i].x < scroll_x + 640)
        {
          if (bad_guys[i].kind == BAD_BSOD)
            {
              /* --- BLUE SCREEN OF DEATH MONSTER: --- */

              if (bad_guys[i].dying == NO)
                {
                  /* Alive: */

                  if (bad_guys[i].dir == LEFT)
                    {
                      drawimage(img_bsod_left[(frame / 5) % 4],
                                bad_guys[i].x - scroll_x,
                                bad_guys[i].y,
                                NO_UPDATE);
                    }
                  else
                    {
                      drawimage(img_bsod_right[(frame / 5) % 4],
                                bad_guys[i].x - scroll_x,
                                bad_guys[i].y,
                                NO_UPDATE);
                    }
                }
              else if (bad_guys[i].dying == FALLING)
                {
                  /* Falling: */

                  if (bad_guys[i].dir == LEFT)
                    {
                      drawimage(img_bsod_falling_left,
                                bad_guys[i].x - scroll_x,
                                bad_guys[i].y,
                                NO_UPDATE);
                    }
                  else
                    {
                      drawimage(img_bsod_falling_right,
                                bad_guys[i].x - scroll_x,
                                bad_guys[i].y,
                                NO_UPDATE);
                    }
                }
              else if (bad_guys[i].dying == SQUISHED)
                {
                  /* Dying - Squished: */

                  if (bad_guys[i].dir == LEFT)
                    {
                      drawimage(img_bsod_squished_left,
                                bad_guys[i].x - scroll_x,
                                bad_guys[i].y + 24,
                                NO_UPDATE);
                    }
                  else
                    {
                      drawimage(img_bsod_squished_right,
                                bad_guys[i].x - scroll_x,
                                bad_guys[i].y + 24,
                                NO_UPDATE);
                    }
                }
            }
          else if (bad_guys[i].kind == BAD_LAPTOP)
            {
              /* --- LAPTOP MONSTER: --- */

              if (bad_guys[i].dying == NO)
                {
                  /* Alive: */

                  if (bad_guys[i].mode == NORMAL)
                    {
                      /* Not flat: */

                      if (bad_guys[i].dir == LEFT)
                        {
                          drawimage(img_laptop_left[(frame / 5) % 3],
                                    bad_guys[i].x - scroll_x,
                                    bad_guys[i].y,
                                    NO_UPDATE);
                        }
                      else
                        {
                          drawimage(img_laptop_right[(frame / 5) % 3],
                                    bad_guys[i].x - scroll_x,
                                    bad_guys[i].y,
                                    NO_UPDATE);
                        }
                    }
                  else
                    {
                      /* Flat: */

                      if (bad_guys[i].dir == LEFT)
                        {
                          drawimage(img_laptop_flat_left,
                                    bad_guys[i].x - scroll_x,
                                    bad_guys[i].y,
                                    NO_UPDATE);
                        }
                      else
                        {
                          drawimage(img_laptop_flat_right,
                                    bad_guys[i].x - scroll_x,
                                    bad_guys[i].y,
                                    NO_UPDATE);
                        }
                    }
                }
              else if (bad_guys[i].dying == FALLING)
                {
                  /* Falling: */

                  if (bad_guys[i].dir == LEFT)
                    {
                      drawimage(img_laptop_falling_left,
                                bad_guys[i].x - scroll_x,
                                bad_guys[i].y,
                                NO_UPDATE);
                    }
                  else
                    {
                      drawimage(img_laptop_falling_right,
                                bad_guys[i].x - scroll_x,
                                bad_guys[i].y,
                                NO_UPDATE);
                    }
                }
            }
          else if (bad_guys[i].kind == BAD_MONEY)
            {
              if (bad_guys[i].ym > -16)
                {
                  if (bad_guys[i].dir == LEFT)
                    {
                      drawimage(img_money_left[0],
                                bad_guys[i].x - scroll_x,
                                bad_guys[i].y,
                                NO_UPDATE);
                    }
                  else
                    {
                      drawimage(img_money_right[0],
                                bad_guys[i].x - scroll_x,
                                bad_guys[i].y,
                                NO_UPDATE);
                    }
                }
              else
                {
                  if (bad_guys[i].dir == LEFT)
                    {
                      drawimage(img_money_left[1],
                                bad_guys[i].x - scroll_x,
                                bad_guys[i].y,
                                NO_UPDATE);
                    }
                  else
                    {
                      drawimage(img_money_right[1],
                                bad_guys[i].x - scroll_x,
                                bad_guys[i].y,
                                NO_UPDATE);
                    }
                }
            }
          else if (bad_guys[i].kind == -1)
          {}
        }
    }


  /* (Tux): */

  if (right == UP && left == UP)
    {
      tux_frame_main = 1;
      tux_frame = 1;
    }
  else
    {
      if ((fire == DOWN && (frame % 2) == 0) ||
          (frame % 4) == 0)
        tux_frame_main = (tux_frame_main + 1) % 4;

      tux_frame = tux_frame_main;

      if (tux_frame == 3)
        tux_frame = 1;
    }


  if (tux_got_coffee && (frame % 2) == 1)
    {
      /* Coffee glow: */

      drawimage(img_red_glow, tux_x - 8, tux_y - 32, NO_UPDATE);
    }


  if (tux_safe == 0 || (frame % 2) == 0)
    {
      if (tux_size == SMALL)
        {
          if (tux_invincible_time)
            {
              /* Draw cape: */

              if (tux_dir == RIGHT)
                {
                  drawimage(cape_right[frame % 2],
                            tux_x, tux_y,
                            NO_UPDATE);
                }
              else
                {
                  drawimage(cape_left[frame % 2],
                            tux_x, tux_y,
                            NO_UPDATE);
                }
            }


          if (tux_dir == RIGHT)
            {
              drawimage(tux_right[tux_frame], tux_x, tux_y, NO_UPDATE);
            }
          else
            {
              drawimage(tux_left[tux_frame], tux_x, tux_y, NO_UPDATE);
            }
        }
      else
        {
          if (tux_invincible_time)
            {
              /* Draw cape: */

              if (tux_dir == RIGHT)
                {
                  drawimage(bigcape_right[frame % 2],
                            tux_x - 8 - 16, tux_y - 32,
                            NO_UPDATE);
                }
              else
                {
                  drawimage(bigcape_left[frame % 2],
                            tux_x - 8, tux_y - 32,
                            NO_UPDATE);
                }
            }

          if (!tux_duck)
            {
              if (!tux_skidding)
                {
                  if (!jumping || tux_ym > 0)
                    {
                      if (tux_dir == RIGHT)
                        {
                          drawimage(bigtux_right[tux_frame],
                                    tux_x - 8, tux_y - 32,
                                    NO_UPDATE);
                        }
                      else
                        {
                          drawimage(bigtux_left[tux_frame],
                                    tux_x - 8, tux_y - 32,
                                    NO_UPDATE);
                        }
                    }
                  else
                    {
                      if (tux_dir == RIGHT)
                        {
                          drawimage(bigtux_right_jump,
                                    tux_x - 8, tux_y - 32,
                                    NO_UPDATE);
                        }
                      else
                        {
                          drawimage(bigtux_left_jump,
                                    tux_x - 8, tux_y - 32,
                                    NO_UPDATE);
                        }
                    }
                }
              else
                {
                  if (tux_dir == RIGHT)
                    {
                      drawimage(skidtux_right,
                                tux_x - 8, tux_y - 32,
                                NO_UPDATE);
                    }
                  else
                    {
                      drawimage(skidtux_left,
                                tux_x - 8, tux_y - 32,
                                NO_UPDATE);
                    }
                }
            }
          else
            {
              if (tux_dir == RIGHT)
                {
                  drawimage(ducktux_right, tux_x - 8, tux_y - 16,
                            NO_UPDATE);
                }
              else
                {
                  drawimage(ducktux_left, tux_x - 8, tux_y - 16,
                            NO_UPDATE);
                }
            }
        }
    }


  /* (Bullets): */

  for (i = 0; i < NUM_BULLETS; i++)
    {
      if (bullets[i].alive &&
          bullets[i].x >= scroll_x - 4 &&
          bullets[i].x <= scroll_x + 640)
        {
          drawimage(img_bullet, bullets[i].x - scroll_x, bullets[i].y,
                    NO_UPDATE);
        }
    }


  /* (Floating scores): */

  for (i = 0; i < NUM_FLOATING_SCORES; i++)
    {
      if (floating_scores[i].alive)
        {
          sprintf(str, "%d", floating_scores[i].value);
          drawtext(str,
                   floating_scores[i].x + 16 - strlen(str) * 8,
                   floating_scores[i].y,
                   letters_gold, NO_UPDATE);
        }
    }


  /* (Upgrades): */

  for (i = 0; i < NUM_UPGRADES; i++)
    {
      if (upgrades[i].alive)
        {
          if (upgrades[i].height < 32)
            {
              /* Rising up... */

              dest.x = upgrades[i].x - scroll_x;
              dest.y = upgrades[i].y + 32 - upgrades[i].height;
              dest.w = 32;
              dest.h = upgrades[i].height;

              src.x = 0;
              src.y = 0;
              src.w = 32;
              src.h = upgrades[i].height;

              if (upgrades[i].kind == UPGRADE_MINTS)
                SDL_BlitSurface(img_mints, &src, screen, &dest);
              else if (upgrades[i].kind == UPGRADE_COFFEE)
                SDL_BlitSurface(img_coffee, &src, screen, &dest);
              else if (upgrades[i].kind == UPGRADE_HERRING)
                SDL_BlitSurface(img_golden_herring, &src, screen, &dest);
            }
          else
            {
              if (upgrades[i].kind == UPGRADE_MINTS)
                {
                  drawimage(img_mints,
                            upgrades[i].x - scroll_x, upgrades[i].y,
                            NO_UPDATE);
                }
              else if (upgrades[i].kind == UPGRADE_COFFEE)
                {
                  drawimage(img_coffee,
                            upgrades[i].x - scroll_x, upgrades[i].y,
                            NO_UPDATE);
                }
              else if (upgrades[i].kind == UPGRADE_HERRING)
                {
                  drawimage(img_golden_herring,
                            upgrades[i].x - scroll_x, upgrades[i].y,
                            NO_UPDATE);
                }
            }
        }
    }


  /* (Bouncy distros): */

  for (i = 0; i < NUM_BOUNCY_DISTROS; i++)
    {
      if (bouncy_distros[i].alive)
        {
          drawimage(img_distro[0],
                    bouncy_distros[i].x - scroll_x,
                    bouncy_distros[i].y,
                    NO_UPDATE);
        }
    }


  /* (Broken bricks): */

  for (i = 0; i < NUM_BROKEN_BRICKS; i++)
    {
      if (broken_bricks[i].alive)
        {
          src.x = rand() % 16;
          src.y = rand() % 16;
          src.w = 16;
          src.h = 16;

          dest.x = broken_bricks[i].x - scroll_x;
          dest.y = broken_bricks[i].y;
          dest.w = 16;
          dest.h = 16;

          SDL_BlitSurface(img_brick[0], &src, screen, &dest);
        }
    }


  /* (Status): */

  sprintf(str, "%d", score);
  drawtext("SCORE", 0, 0, letters_blue, NO_UPDATE);
  drawtext(str, 96, 0, letters_gold, NO_UPDATE);

  sprintf(str, "%d", highscore);
  drawtext("HIGH", 0, 20, letters_blue, NO_UPDATE);
  drawtext(str, 96, 20, letters_gold, NO_UPDATE);

  if (time_left >= 50 || (frame % 10) < 5)
    {
      sprintf(str, "%d", time_left);
      drawtext("TIME", 224, 0, letters_blue, NO_UPDATE);
      drawtext(str, 304, 0, letters_gold, NO_UPDATE);
    }

  sprintf(str, "%d", distros);
  drawtext("DISTROS", 480, 0, letters_blue, NO_UPDATE);
  drawtext(str, 608, 0, letters_gold, NO_UPDATE);

  if(game_pause)
    drawcenteredtext("PAUSE",230,letters_red, NO_UPDATE);

  if(show_menu)
    done = drawmenu();

  /* (Update it all!) */

  updatescreen();


}

/* --- GAME LOOP! --- */

int gameloop(void)
{

  Uint32 last_time, now_time;

  /* Clear screen: */

  clearscreen(0, 0, 0);
  updatescreen();


  /* Init the game: */

  initmenu();
  initgame();
  loadshared();
  loadlevel();
  loadlevelgfx();
  loadlevelsong();
  highscore = load_hs();


  /* --- MAIN GAME LOOP!!! --- */

  done = 0;
  quit = 0;
  frame = 0;
  tux_frame_main = 0;
  tux_frame = 0;
  game_pause = 0;

  game_draw();
  do
    {
      last_time = SDL_GetTicks();
      frame++;


      /* Handle events: */

      old_fire = fire;

      game_event();

      /* Handle actions: */

      if(!game_pause && !show_menu)
        {
          if (game_action() == 0)
            {
              /* == 0: no more lives */
              /* == -1: continues */
              return 0;
            }
        }
      else
        SDL_Delay(50);

      /*Draw the current scene to the screen */
      game_draw();

      /* Keep playing music: */


      if (!playing_music())
        {
          switch (current_music)
            {
            case LEVEL_MUSIC:
              play_music(level_song, 1);
              break;
            case HERRING_MUSIC:
              play_music(herring_song, 1);
              break;
            case HURRYUP_MUSIC: // keep the compiler happy
            case NO_MUSIC:      // keep the compiler happy for the moment :-)
            {}
              /*default:*/
            }
        }

      /* Time stops in pause mode */
      if(game_pause || show_menu )
        {
          continue;
        }

      /* Pause til next frame: */

      now_time = SDL_GetTicks();
      if (now_time < last_time + FPS)
        SDL_Delay(last_time + FPS - now_time);


      /* Handle time: */

      if ((frame % 10) == 0 && time_left > 0)
        {
          time_left--;

          if (time_left <= 0)
            killtux(KILL);
        }
    }
  while (!done && !quit);

  if (playing_music())
    halt_music();

  unloadlevelgfx();
  unloadlevelsong();
  unloadshared();

  return(quit);
}


/* Initialize the game stuff: */

void initgame(void)
{
  level = 1;
  score = 0;
  distros = 0;
  lives = 3;
}



/* Load data for this level: */

void loadlevel(void)
{
  int i, x, y;
  FILE * fi;
  char * filename;
  char str[80];
  char * line;


  /* Reset arrays: */

  for (i = 0; i < NUM_BOUNCY_DISTROS; i++)
    bouncy_distros[i].alive = NO;

  for (i = 0; i < NUM_BROKEN_BRICKS; i++)
    broken_bricks[i].alive = NO;

  for (i = 0; i < NUM_BOUNCY_BRICKS; i++)
    bouncy_bricks[i].alive = NO;

  for (i = 0; i < NUM_BAD_GUYS; i++)
    bad_guys[i].alive = NO;

  for (i = 0; i < NUM_FLOATING_SCORES; i++)
    floating_scores[i].alive = NO;

  for (i = 0; i < NUM_UPGRADES; i++)
    upgrades[i].alive = NO;

  for (i = 0; i < NUM_BULLETS; i++)
    bullets[i].alive = NO;


  /* Load data file: */

  filename = (char *) malloc(sizeof(char) * (strlen(DATA_PREFIX) + 20));
  sprintf(filename, "%s/levels/level%d.dat", DATA_PREFIX, level);
  fi = fopen(filename, "r");
  if (fi == NULL)
    {
      perror(filename);
      st_shutdown();
      free(filename);
      exit(-1);
    }
  free(filename);


  /* Load header info: */


  /* (Level title) */
  fgets(str, 20, fi);
  strcpy(levelname, str);
  levelname[strlen(levelname)-1] = '\0';

  /* (Level theme) */
  fgets(str, 20, fi);
  strcpy(leveltheme, str);
  leveltheme[strlen(leveltheme)-1] = '\0';



  /* (Time to beat level) */
  fgets(str, 10, fi);
  time_left = atoi(str);

  /* (Song file for this level) */
  fgets(str, 20, fi);
  strcpy(song_title, str);
  song_title[strlen(song_title)-1] = '\0';



  /* (Level background color) */
  fgets(str, 10, fi);
  bkgd_red = atoi(str);
  fgets(str, 10, fi);
  bkgd_green= atoi(str);
  fgets(str, 10, fi);
  bkgd_blue = atoi(str);

  /* (Level width) */
  fgets(str, 10, fi);
  level_width = atoi(str);


  /* Allocate some space for the line-reading! */

  line = (char *) malloc(sizeof(char) * (level_width + 5));
  if (line == NULL)
    {
      fprintf(stderr, "Couldn't allocate space to load level data!");
      exit(1);
    }


  /* Load the level lines: */

  for (y = 0; y < 15; y++)
    {
      if(fgets(line, level_width + 5, fi) == NULL)
        {
          fprintf(stderr, "Level %s isn't complete!\n",levelname);
          exit(1);
        }
      line[strlen(line) - 1] = '\0';
      tiles[y] = strdup(line);
    }

  fclose(fi);


  /* Activate bad guys: */

  for (y = 0; y < 15; y++)
    {
      for (x = 0; x < level_width; x++)
        {
          if (tiles[y][x] >= '0' && tiles[y][x] <= '9')
            {
              add_bad_guy(x * 32, y * 32, tiles[y][x] - '0');
              tiles[y][x] = '.';
            }
        }
    }


  /* Set defaults: */

  tux_x = 0;
  tux_xm = 0;
  tux_y = 240;
  tux_ym = 0;
  tux_dir = RIGHT;
  tux_size = SMALL;
  tux_got_coffee = NO;
  tux_invincible_time = 0;
  tux_duck = NO;

  tux_dying = NO;
  tux_safe = TUX_SAFE_TIME;

  jumping = NO;
  jump_counter = 0;

  tux_skidding = 0;

  scroll_x = 0;

  right = UP;
  left = UP;
  up = UP;
  down = UP;
  fire = UP;
  old_fire = UP;

  score_multiplier = 1;
  super_bkgd_time = 0;

  counting_distros = NO;
  distro_counter = 0;

  endpos = 0;

  /* set current song/music */
  current_music = LEVEL_MUSIC;

  /* Level Intro: */

  clearscreen(0, 0, 0);

  sprintf(str, "LEVEL %d", level);
  drawcenteredtext(str, 200, letters_red, NO_UPDATE);

  sprintf(str, "%s", levelname);
  drawcenteredtext(str, 224, letters_gold, NO_UPDATE);

  sprintf(str, "TUX x %d", lives);
  drawcenteredtext(str, 256, letters_blue, NO_UPDATE);

  SDL_Flip(screen);

  SDL_Delay(1000);


}


/* Load a level-specific graphic... */

SDL_Surface * load_level_image(char * file, int use_alpha)
{
  char fname[1024];

  snprintf(fname, 1024, "%s/images/%s/%s", DATA_PREFIX, leveltheme, file);

  return(load_image(fname, use_alpha));
}


/* Load graphics: */

void loadlevelgfx(void)
{
  img_brick[0] = load_level_image("brick0.png", IGNORE_ALPHA);
  img_brick[1] = load_level_image("brick1.png", IGNORE_ALPHA);

  img_solid[0] = load_level_image("solid0.png", USE_ALPHA);
  img_solid[1] = load_level_image("solid1.png", USE_ALPHA);
  img_solid[2] = load_level_image("solid2.png", USE_ALPHA);
  img_solid[3] = load_level_image("solid3.png", USE_ALPHA);

  img_bkgd[0][0] = load_level_image("bkgd-00.png", USE_ALPHA);
  img_bkgd[0][1] = load_level_image("bkgd-01.png", USE_ALPHA);
  img_bkgd[0][2] = load_level_image("bkgd-02.png", USE_ALPHA);
  img_bkgd[0][3] = load_level_image("bkgd-03.png", USE_ALPHA);

  img_bkgd[1][0] = load_level_image("bkgd-10.png", USE_ALPHA);
  img_bkgd[1][1] = load_level_image("bkgd-11.png", USE_ALPHA);
  img_bkgd[1][2] = load_level_image("bkgd-12.png", USE_ALPHA);
  img_bkgd[1][3] = load_level_image("bkgd-13.png", USE_ALPHA);
}


/* Load music: */

void loadlevelsong(void)
{

  char * song_path;

  song_path = (char *) malloc(sizeof(char) * (strlen(DATA_PREFIX) +
                              strlen(song_title) + 8));

  sprintf(song_path, "%s/music/%s", DATA_PREFIX, song_title);

  level_song = load_song(song_path);

  free(song_path);
}


/* Free graphics data for this level: */

void unloadlevelgfx(void)
{
  int i;

  for (i = 0; i < 2; i++)
    {
      SDL_FreeSurface(img_brick[i]);
    }
  for (i = 0; i < 4; i++)
    {
      SDL_FreeSurface(img_solid[i]);
      SDL_FreeSurface(img_bkgd[0][i]);
      SDL_FreeSurface(img_bkgd[1][i]);
    }
}


/* Free music data for this level: */

void unloadlevelsong(void)
{
  free_music(level_song);
}


/* Load graphics/sounds shared between all levels: */

void loadshared(void)
{
  int i;
  char * herring_song_path; /* for loading herring song*/

  /* Tuxes: */

  tux_right[0] = load_image(DATA_PREFIX "/images/shared/tux-right-0.png",
                            USE_ALPHA);

  tux_right[1] = load_image(DATA_PREFIX "/images/shared/tux-right-1.png",
                            USE_ALPHA);

  tux_right[2] = load_image(DATA_PREFIX "/images/shared/tux-right-2.png",
                            USE_ALPHA);

  tux_left[0] = load_image(DATA_PREFIX "/images/shared/tux-left-0.png",
                           USE_ALPHA);

  tux_left[1] = load_image(DATA_PREFIX "/images/shared/tux-left-1.png",
                           USE_ALPHA);

  tux_left[2] = load_image(DATA_PREFIX "/images/shared/tux-left-2.png",
                           USE_ALPHA);

  cape_right[0] = load_image(DATA_PREFIX "/images/shared/cape-right-0.png",
                             USE_ALPHA);

  cape_right[1] = load_image(DATA_PREFIX "/images/shared/cape-right-1.png",
                             USE_ALPHA);

  cape_left[0] = load_image(DATA_PREFIX "/images/shared/cape-left-0.png",
                            USE_ALPHA);

  cape_left[1] = load_image(DATA_PREFIX "/images/shared/cape-left-1.png",
                            USE_ALPHA);

  bigtux_right[0] = load_image(DATA_PREFIX "/images/shared/bigtux-right-0.png",
                               USE_ALPHA);

  bigtux_right[1] = load_image(DATA_PREFIX "/images/shared/bigtux-right-1.png",
                               USE_ALPHA);

  bigtux_right[2] = load_image(DATA_PREFIX "/images/shared/bigtux-right-2.png",
                               USE_ALPHA);

  bigtux_right_jump =
    load_image(DATA_PREFIX "/images/shared/bigtux-right-jump.png", USE_ALPHA);

  bigtux_left[0] = load_image(DATA_PREFIX "/images/shared/bigtux-left-0.png",
                              USE_ALPHA);

  bigtux_left[1] = load_image(DATA_PREFIX "/images/shared/bigtux-left-1.png",
                              USE_ALPHA);

  bigtux_left[2] = load_image(DATA_PREFIX "/images/shared/bigtux-left-2.png",
                              USE_ALPHA);

  bigtux_left_jump =
    load_image(DATA_PREFIX "/images/shared/bigtux-left-jump.png", USE_ALPHA);

  bigcape_right[0] =
    load_image(DATA_PREFIX "/images/shared/bigcape-right-0.png",
               USE_ALPHA);

  bigcape_right[1] =
    load_image(DATA_PREFIX "/images/shared/bigcape-right-1.png",
               USE_ALPHA);

  bigcape_left[0] =
    load_image(DATA_PREFIX "/images/shared/bigcape-left-0.png",
               USE_ALPHA);

  bigcape_left[1] =
    load_image(DATA_PREFIX "/images/shared/bigcape-left-1.png",
               USE_ALPHA);

  ducktux_right = load_image(DATA_PREFIX
                             "/images/shared/ducktux-right.png",
                             USE_ALPHA);

  ducktux_left = load_image(DATA_PREFIX
                            "/images/shared/ducktux-left.png",
                            USE_ALPHA);

  skidtux_right = load_image(DATA_PREFIX
                             "/images/shared/skidtux-right.png",
                             USE_ALPHA);

  skidtux_left = load_image(DATA_PREFIX
                            "/images/shared/skidtux-left.png",
                            USE_ALPHA);


  /* Boxes: */

  img_box_full = load_image(DATA_PREFIX "/images/shared/box-full.png",
                            IGNORE_ALPHA);
  img_box_empty = load_image(DATA_PREFIX "/images/shared/box-empty.png",
                             IGNORE_ALPHA);


  /* Water: */


  img_water = load_image(DATA_PREFIX "/images/shared/water.png", IGNORE_ALPHA);

  img_waves[0] = load_image(DATA_PREFIX "/images/shared/waves-0.png",
                            USE_ALPHA);

  img_waves[1] = load_image(DATA_PREFIX "/images/shared/waves-1.png",
                            USE_ALPHA);

  img_waves[2] = load_image(DATA_PREFIX "/images/shared/waves-2.png",
                            USE_ALPHA);


  /* Pole: */

  img_pole = load_image(DATA_PREFIX "/images/shared/pole.png", USE_ALPHA);
  img_poletop = load_image(DATA_PREFIX "/images/shared/poletop.png",
                           USE_ALPHA);


  /* Flag: */

  img_flag[0] = load_image(DATA_PREFIX "/images/shared/flag-0.png",
                           USE_ALPHA);
  img_flag[1] = load_image(DATA_PREFIX "/images/shared/flag-1.png",
                           USE_ALPHA);


  /* Cloud: */

  img_cloud[0][0] = load_image(DATA_PREFIX "/images/shared/cloud-00.png",
                               USE_ALPHA);

  img_cloud[0][1] = load_image(DATA_PREFIX "/images/shared/cloud-01.png",
                               USE_ALPHA);

  img_cloud[0][2] = load_image(DATA_PREFIX "/images/shared/cloud-02.png",
                               USE_ALPHA);

  img_cloud[0][3] = load_image(DATA_PREFIX "/images/shared/cloud-03.png",
                               USE_ALPHA);


  img_cloud[1][0] = load_image(DATA_PREFIX "/images/shared/cloud-10.png",
                               USE_ALPHA);

  img_cloud[1][1] = load_image(DATA_PREFIX "/images/shared/cloud-11.png",
                               USE_ALPHA);

  img_cloud[1][2] = load_image(DATA_PREFIX "/images/shared/cloud-12.png",
                               USE_ALPHA);

  img_cloud[1][3] = load_image(DATA_PREFIX "/images/shared/cloud-13.png",
                               USE_ALPHA);


  /* Bad guys: */

  /* (BSOD) */

  img_bsod_left[0] = load_image(DATA_PREFIX
                                "/images/shared/bsod-left-0.png",
                                USE_ALPHA);

  img_bsod_left[1] = load_image(DATA_PREFIX
                                "/images/shared/bsod-left-1.png",
                                USE_ALPHA);

  img_bsod_left[2] = load_image(DATA_PREFIX
                                "/images/shared/bsod-left-2.png",
                                USE_ALPHA);

  img_bsod_left[3] = load_image(DATA_PREFIX
                                "/images/shared/bsod-left-3.png",
                                USE_ALPHA);

  img_bsod_right[0] = load_image(DATA_PREFIX
                                 "/images/shared/bsod-right-0.png",
                                 USE_ALPHA);

  img_bsod_right[1] = load_image(DATA_PREFIX
                                 "/images/shared/bsod-right-1.png",
                                 USE_ALPHA);

  img_bsod_right[2] = load_image(DATA_PREFIX
                                 "/images/shared/bsod-right-2.png",
                                 USE_ALPHA);

  img_bsod_right[3] = load_image(DATA_PREFIX
                                 "/images/shared/bsod-right-3.png",
                                 USE_ALPHA);

  img_bsod_squished_left = load_image(DATA_PREFIX
                                      "/images/shared/bsod-squished-left.png",
                                      USE_ALPHA);

  img_bsod_squished_right = load_image(DATA_PREFIX
                                       "/images/shared/bsod-squished-right.png",
                                       USE_ALPHA);

  img_bsod_falling_left = load_image(DATA_PREFIX
                                     "/images/shared/bsod-falling-left.png",
                                     USE_ALPHA);

  img_bsod_falling_right = load_image(DATA_PREFIX
                                      "/images/shared/bsod-falling-right.png",
                                      USE_ALPHA);


  /* (Laptop) */

  img_laptop_left[0] = load_image(DATA_PREFIX
                                  "/images/shared/laptop-left-0.png",
                                  USE_ALPHA);

  img_laptop_left[1] = load_image(DATA_PREFIX
                                  "/images/shared/laptop-left-1.png",
                                  USE_ALPHA);

  img_laptop_left[2] = load_image(DATA_PREFIX
                                  "/images/shared/laptop-left-2.png",
                                  USE_ALPHA);

  img_laptop_right[0] = load_image(DATA_PREFIX
                                   "/images/shared/laptop-right-0.png",
                                   USE_ALPHA);

  img_laptop_right[1] = load_image(DATA_PREFIX
                                   "/images/shared/laptop-right-1.png",
                                   USE_ALPHA);

  img_laptop_right[2] = load_image(DATA_PREFIX
                                   "/images/shared/laptop-right-2.png",
                                   USE_ALPHA);

  img_laptop_flat_left = load_image(DATA_PREFIX
                                    "/images/shared/laptop-flat-left.png",
                                    USE_ALPHA);

  img_laptop_flat_right = load_image(DATA_PREFIX
                                     "/images/shared/laptop-flat-right.png",
                                     USE_ALPHA);

  img_laptop_falling_left =
    load_image(DATA_PREFIX
               "/images/shared/laptop-falling-left.png",
               USE_ALPHA);

  img_laptop_falling_right =
    load_image(DATA_PREFIX
               "/images/shared/laptop-falling-right.png",
               USE_ALPHA);


  /* (Money) */

  img_money_left[0] = load_image(DATA_PREFIX
                                 "/images/shared/bag-left-0.png",
                                 USE_ALPHA);

  img_money_left[1] = load_image(DATA_PREFIX
                                 "/images/shared/bag-left-1.png",
                                 USE_ALPHA);

  img_money_right[0] = load_image(DATA_PREFIX
                                  "/images/shared/bag-right-0.png",
                                  USE_ALPHA);

  img_money_right[1] = load_image(DATA_PREFIX
                                  "/images/shared/bag-right-1.png",
                                  USE_ALPHA);



  /* Upgrades: */

  img_mints = load_image(DATA_PREFIX "/images/shared/mints.png", USE_ALPHA);
  img_coffee = load_image(DATA_PREFIX "/images/shared/coffee.png", USE_ALPHA);


  /* Weapons: */

  img_bullet = load_image(DATA_PREFIX "/images/shared/bullet.png", USE_ALPHA);

  img_red_glow = load_image(DATA_PREFIX "/images/shared/red-glow.png",
                            USE_ALPHA);


  /* Distros: */

  img_distro[0] = load_image(DATA_PREFIX "/images/shared/distro-0.png",
                             USE_ALPHA);

  img_distro[1] = load_image(DATA_PREFIX "/images/shared/distro-1.png",
                             USE_ALPHA);

  img_distro[2] = load_image(DATA_PREFIX "/images/shared/distro-2.png",
                             USE_ALPHA);

  img_distro[3] = load_image(DATA_PREFIX "/images/shared/distro-3.png",
                             USE_ALPHA);


  /* Herring: */

  img_golden_herring =
    load_image(DATA_PREFIX "/images/shared/golden-herring.png",
               USE_ALPHA);


  /* Super background: */

  img_super_bkgd = load_image(DATA_PREFIX "/images/shared/super-bkgd.png",
                              IGNORE_ALPHA);


  /* Sound effects: */

  /*  if (use_sound) // this can help speeding up a little, but
     we shouldn't take care about "use_sound" here, it's load_sound's job
     / Send a mail to neoneurone@users.sf.net, if you have another opinion. :)
     */
  for (i = 0; i < NUM_SOUNDS; i++)
    sounds[i] = load_sound(soundfilenames[i]);

  /* Herring song */
  herring_song_path = (char *) malloc(sizeof(char) * (strlen(DATA_PREFIX) +
                                      strlen("SALCON.MOD") + 8)); /* FIXME: We need a real herring_song! Thats a fake.:) */

  sprintf(herring_song_path, "%s/music/%s", DATA_PREFIX, "SALCON.MOD");

  herring_song = load_song(herring_song_path);

  free(herring_song_path);

}


/* Free shared data: */

void unloadshared(void)
{
  int i;

  for (i = 0; i < 3; i++)
    {
      SDL_FreeSurface(tux_right[i]);
      SDL_FreeSurface(tux_left[i]);
      SDL_FreeSurface(bigtux_right[i]);
      SDL_FreeSurface(bigtux_left[i]);
    }

  SDL_FreeSurface(bigtux_right_jump);
  SDL_FreeSurface(bigtux_left_jump);

  for (i = 0; i < 2; i++)
    {
      SDL_FreeSurface(cape_right[i]);
      SDL_FreeSurface(cape_left[i]);
      SDL_FreeSurface(bigcape_right[i]);
      SDL_FreeSurface(bigcape_left[i]);
    }

  SDL_FreeSurface(ducktux_left);
  SDL_FreeSurface(ducktux_right);

  SDL_FreeSurface(skidtux_left);
  SDL_FreeSurface(skidtux_right);

  for (i = 0; i < 4; i++)
    {
      SDL_FreeSurface(img_bsod_left[i]);
      SDL_FreeSurface(img_bsod_right[i]);
    }

  SDL_FreeSurface(img_bsod_squished_left);
  SDL_FreeSurface(img_bsod_squished_right);

  SDL_FreeSurface(img_bsod_falling_left);
  SDL_FreeSurface(img_bsod_falling_right);

  for (i = 0; i < 3; i++)
    {
      SDL_FreeSurface(img_laptop_left[i]);
      SDL_FreeSurface(img_laptop_right[i]);
    }

  SDL_FreeSurface(img_laptop_flat_left);
  SDL_FreeSurface(img_laptop_flat_right);

  SDL_FreeSurface(img_laptop_falling_left);
  SDL_FreeSurface(img_laptop_falling_right);

  for (i = 0; i < 2; i++)
    {
      SDL_FreeSurface(img_money_left[i]);
      SDL_FreeSurface(img_money_right[i]);
    }

  SDL_FreeSurface(img_box_full);
  SDL_FreeSurface(img_box_empty);

  SDL_FreeSurface(img_water);
  for (i = 0; i < 3; i++)
    SDL_FreeSurface(img_waves[i]);

  SDL_FreeSurface(img_pole);
  SDL_FreeSurface(img_poletop);

  for (i = 0; i < 2; i++)
    SDL_FreeSurface(img_flag[i]);

  SDL_FreeSurface(img_mints);
  SDL_FreeSurface(img_coffee);

  for (i = 0; i < 4; i++)
    {
      SDL_FreeSurface(img_distro[i]);
      SDL_FreeSurface(img_cloud[0][i]);
      SDL_FreeSurface(img_cloud[1][i]);
    }

  SDL_FreeSurface(img_golden_herring);

  for (i = 0; i < NUM_SOUNDS; i++)
    free_chunk(sounds[i]);

  /* free the herring song */
  free_music( herring_song );
}


/* Draw a tile on the screen: */

void drawshape(int x, int y, unsigned char c)
{
  int z;

  if (c == 'X' || c == 'x')
    drawimage(img_brick[0], x, y, NO_UPDATE);
  else if (c == 'Y' || c == 'y')
    drawimage(img_brick[1], x, y, NO_UPDATE);
  else if (c == 'A' || c =='B' || c == '!')
    drawimage(img_box_full, x, y, NO_UPDATE);
  else if (c == 'a')
    drawimage(img_box_empty, x, y, NO_UPDATE);
  else if (c >= 'C' && c <= 'F')
    drawimage(img_cloud[0][c - 'C'], x, y, NO_UPDATE);
  else if (c >= 'c' && c <= 'f')
    drawimage(img_cloud[1][c - 'c'], x, y, NO_UPDATE);
  else if (c >= 'G' && c <= 'J')
    drawimage(img_bkgd[0][c - 'G'], x, y, NO_UPDATE);
  else if (c >= 'g' && c <= 'j')
    drawimage(img_bkgd[1][c - 'g'], x, y, NO_UPDATE);
  else if (c == '#')
    drawimage(img_solid[0], x, y, NO_UPDATE);
  else if (c == '[')
    drawimage(img_solid[1], x, y, NO_UPDATE);
  else if (c == '=')
    drawimage(img_solid[2], x, y, NO_UPDATE);
  else if (c == ']')
    drawimage(img_solid[3], x, y, NO_UPDATE);
  else if (c == '$')
    {
      z = (frame / 2) % 6;

      if (z < 4)
        drawimage(img_distro[z], x, y, NO_UPDATE);
      else if (z == 4)
        drawimage(img_distro[2], x, y, NO_UPDATE);
      else if (z == 5)
        drawimage(img_distro[1], x, y, NO_UPDATE);
    }
  else if (c == '^')
    {
      z = (frame / 3) % 3;

      drawimage(img_waves[z], x, y, NO_UPDATE);
    }
  else if (c == '*')
    drawimage(img_poletop, x, y, NO_UPDATE);
  else if (c == '|')
    {
      drawimage(img_pole, x, y, NO_UPDATE);

      /* Mark this as the end position of the level! */

      endpos = x;
    }
  else if (c == '\\')
    {
      z = (frame / 3) % 2;

      drawimage(img_flag[z], x + 16, y, NO_UPDATE);
    }
  else if (c == '&')
    drawimage(img_water, x, y, NO_UPDATE);
}


/* What shape is at some position? */

unsigned char shape(int x, int y, int sx)
{
  int xx, yy;
  unsigned char c;

  yy = (y / 32);
  xx = ((x + sx) / 32);

  if (yy >= 0 && yy <= 15 && xx >= 0 && xx <= level_width)
    c = tiles[yy][xx];
  else
    c = '.';

  return(c);
}


/* Is is ground? */

int issolid(int x, int y, int sx)
{
  int v;

  v = 0;

  if (isbrick(x, y, sx) ||
      isbrick(x + 31, y, sx) ||
      isice(x, y, sx) ||
      isice(x + 31, y, sx) ||
      (shape(x, y, sx) == '[' ||
       shape(x + 31, y, sx) == '[') ||
      (shape(x, y, sx) == '=' ||
       shape(x + 31, y, sx) == '=') ||
      (shape(x, y, sx) == ']' ||
       shape(x + 31, y, sx) == ']') ||
      (shape(x, y, sx) == 'A' ||
       shape(x + 31, y, sx) == 'A') ||
      (shape(x, y, sx) == 'B' ||
       shape(x + 31, y, sx) == 'B') ||
      (shape(x, y, sx) == '!' ||
       shape(x + 31, y, sx) == '!') ||
      (shape(x, y, sx) == 'a' ||
       shape(x + 31, y, sx) == 'a'))
    {
      v = 1;
    }

  return(v);
}


/* Is it a brick? */

int isbrick(int x, int y, int sx)
{
  int v;

  v = 0;

  if (shape(x, y, sx) == 'X' ||
      shape(x, y, sx) == 'x' ||
      shape(x, y, sx) == 'Y' ||
      shape(x, y, sx) == 'y')
    {
      v = 1;
    }

  return(v);
}


/* Is it ice? */

int isice(int x, int y, int sx)
{
  int v;

  v = 0;

  if (shape(x, y, sx) == '#')
    {
      v = 1;
    }

  return(v);
}


/* Is it a full box? */

int isfullbox(int x, int y, int sx)
{
  int v;

  v = 0;

  if (shape(x, y, sx) == 'A' ||
      shape(x, y, sx) == 'B' ||
      shape(x, y, sx) == '!')
    {
      v = 1;
    }

  return(v);
}


/* Edit a piece of the map! */

void change(int x, int y, int sx, unsigned char c)
{
  int xx, yy;

  yy = (y / 32);
  xx = ((x + sx) / 32);

  if (yy >= 0 && yy <= 15 && xx >= 0 && xx <= level_width)
    tiles[yy][xx] = c;
}


/* Break a brick: */

void trybreakbrick(int x, int y, int sx)
{
  if (isbrick(x, y, sx))
    {
      if (shape(x, y, sx) == 'x' || shape(x, y, sx) == 'y')
        {
          /* Get a distro from it: */

          add_bouncy_distro(((x + sx + 1) / 32) * 32,
                            (y / 32) * 32);

          if (counting_distros == NO)
            {
              counting_distros = YES;
              distro_counter = 50;
            }

          if (distro_counter <= 0)
            change(x, y, sx, 'a');

          play_sound(sounds[SND_DISTRO]);
          score = score + SCORE_DISTRO;
          distros++;
        }
      else
        {
          /* Get rid of it: */

          change(x, y, sx, '.');
        }


      /* Replace it with broken bits: */

      add_broken_brick(((x + sx + 1) / 32) * 32,
                       (y / 32) * 32);


      /* Get some score: */

      play_sound(sounds[SND_BRICK]);
      score = score + SCORE_BRICK;
    }
}


/* Bounce a brick: */

void bumpbrick(int x, int y, int sx)
{
  add_bouncy_brick(((x + sx + 1) / 32) * 32,
                   (y / 32) * 32);

  play_sound(sounds[SND_BRICK]);
}


/* Empty a box: */

void tryemptybox(int x, int y, int sx)
{
  if (isfullbox(x, y, sx))
    {
      if (shape(x, y, sx) == 'A')
        {
          /* Box with a distro! */

          add_bouncy_distro(((x + sx + 1) / 32) * 32,
                            (y / 32) * 32 - 32);

          play_sound(sounds[SND_DISTRO]);
          score = score + SCORE_DISTRO;
          distros++;
        }
      else if (shape(x, y, sx) == 'B')
        {
          /* Add an upgrade! */

          if (tux_size == SMALL)
            {
              /* Tux is small, add mints! */

              add_upgrade(((x + sx + 1) / 32) * 32,
                          (y / 32) * 32 - 32,
                          UPGRADE_MINTS);
            }
          else
            {
              /* Tux is big, add coffee: */

              add_upgrade(((x + sx + 1) / 32) * 32,
                          (y / 32) * 32 - 32,
                          UPGRADE_COFFEE);
            }

          play_sound(sounds[SND_UPGRADE]);
        }
      else if (shape(x, y, sx) == '!')
        {
          /* Add a golden herring */

          add_upgrade(((x + sx + 1) / 32) * 32,
                      (y / 32) * 32 - 32,
                      UPGRADE_HERRING);
        }

      /* Empty the box: */

      change(x, y, sx, 'a');
    }
}


/* Try to grab a distro: */

void trygrabdistro(int x, int y, int sx, int bounciness)
{
  if (shape(x, y, sx) == '$')
    {
      change(x, y, sx, '.');
      play_sound(sounds[SND_DISTRO]);

      if (bounciness == BOUNCE)
        {
          add_bouncy_distro(((x + sx + 1) / 32) * 32,
                            (y / 32) * 32);
        }

      score = score + SCORE_DISTRO;
      distros++;
    }
}


/* Add a bouncy distro: */

void add_bouncy_distro(int x, int y)
{
  int i, found;

  found = -1;

  for (i = 0; i < NUM_BOUNCY_DISTROS && found == -1; i++)
    {
      if (!bouncy_distros[i].alive)
        found = i;
    }

  if (found != -1)
    {
      bouncy_distros[found].alive = YES;
      bouncy_distros[found].x = x;
      bouncy_distros[found].y = y;
      bouncy_distros[found].ym = -6;
    }
}


/* Add broken brick pieces: */

void add_broken_brick(int x, int y)
{
  add_broken_brick_piece(x, y, -4, -16);
  add_broken_brick_piece(x, y + 16, -6, -12);

  add_broken_brick_piece(x + 16, y, 4, -16);
  add_broken_brick_piece(x + 16, y + 16, 6, -12);
}


/* Add a broken brick piece: */

void add_broken_brick_piece(int x, int y, int xm, int ym)
{
  int i, found;

  found = -1;

  for (i = 0; i < NUM_BROKEN_BRICKS && found == -1; i++)
    {
      if (!broken_bricks[i].alive)
        found = i;
    }

  if (found != -1)
    {
      broken_bricks[found].alive = YES;
      broken_bricks[found].x = x;
      broken_bricks[found].y = y;
      broken_bricks[found].xm = xm;
      broken_bricks[found].ym = ym;
    }
}


/* Add a bouncy brick piece: */

void add_bouncy_brick(int x, int y)
{
  int i, found;

  found = -1;

  for (i = 0; i < NUM_BOUNCY_BRICKS && found == -1; i++)
    {
      if (!bouncy_bricks[i].alive)
        found = i;
    }

  if (found != -1)
    {
      bouncy_bricks[found].alive = YES;
      bouncy_bricks[found].x = x;
      bouncy_bricks[found].y = y;
      bouncy_bricks[found].offset = 0;
      bouncy_bricks[found].offset_m = -BOUNCY_BRICK_SPEED;
      bouncy_bricks[found].shape = shape(x, y, 0);
    }
}


/* Add a bad guy: */

void add_bad_guy(int x, int y, int kind)
{
  int i, found;

  found = -1;

  for (i = 0; i < NUM_BAD_GUYS && found == -1; i++)
    {
      if (!bad_guys[i].alive)
        found = i;
    }

  if (found != -1)
    {
      bad_guys[found].alive = YES;
      bad_guys[found].mode = NORMAL;
      bad_guys[found].dying = NO;
      bad_guys[found].timer = 0;
      bad_guys[found].kind = kind;
      bad_guys[found].x = x;
      bad_guys[found].y = y;
      bad_guys[found].xm = 0;
      bad_guys[found].ym = 0;
      bad_guys[found].dir = LEFT;
      bad_guys[found].seen = NO;
    }
}


/* Add score: */

void add_score(int x, int y, int s)
{
  int i, found;


  /* Add the score: */

  score = score + s;


  /* Add a floating score thing to the game: */

  found = -1;

  for (i = 0; i < NUM_FLOATING_SCORES && found == -1; i++)
    {
      if (!floating_scores[i].alive)
        found = i;
    }


  if (found != -1)
    {
      floating_scores[found].alive = YES;
      floating_scores[found].x = x;
      floating_scores[found].y = y - 16;
      floating_scores[found].timer = 8;
      floating_scores[found].value = s;
    }
}


/* Try to bump a bad guy from below: */

void trybumpbadguy(int x, int y, int sx)
{
  int i;


  /* Bad guys: */

  for (i = 0; i < NUM_BAD_GUYS; i++)
    {
      if (bad_guys[i].alive &&
          bad_guys[i].x >= x + sx - 32 && bad_guys[i].x <= x + sx + 32 &&
          bad_guys[i].y >= y - 16 && bad_guys[i].y <= y + 16)
        {
          if (bad_guys[i].kind == BAD_BSOD ||
              bad_guys[i].kind == BAD_LAPTOP)
            {
              bad_guys[i].dying = FALLING;
              bad_guys[i].ym = -8;
              play_sound(sounds[SND_FALL]);
            }
        }
    }


  /* Upgrades: */

  for (i = 0; i < NUM_UPGRADES; i++)
    {
      if (upgrades[i].alive && upgrades[i].height == 32 &&
          upgrades[i].x >= x + sx - 32 && upgrades[i].x <= x + sx + 32 &&
          upgrades[i].y >= y - 16 && upgrades[i].y <= y + 16)
        {
          upgrades[i].xm = -upgrades[i].xm;
          upgrades[i].ym = -8;
          play_sound(sounds[SND_BUMP_UPGRADE]);
        }
    }
}


/* Add an upgrade: */

void add_upgrade(int x, int y, int kind)
{
  int i, found;

  found = -1;

  for (i = 0; i < NUM_UPGRADES && found == -1; i++)
    {
      if (!upgrades[i].alive)
        found = i;
    }

  if (found != -1)
    {
      upgrades[found].alive = YES;
      upgrades[found].kind = kind;
      upgrades[found].x = x;
      upgrades[found].y = y;
      upgrades[found].xm = 4;
      upgrades[found].ym = -4;
      upgrades[found].height = 0;
    }
}


/* Kill tux! */

void killtux(int mode)
{
  tux_ym = -16;

  play_sound(sounds[SND_HURT]);

  if (tux_dir == RIGHT)
    tux_xm = -8;
  else if (tux_dir == LEFT)
    tux_xm = 8;

  if (mode == SHRINK && tux_size == BIG)
    {
      if (tux_got_coffee)
        tux_got_coffee = NO;

      tux_size = SMALL;

      tux_safe = TUX_SAFE_TIME;
    }
  else
    {
      tux_dying = 1;
    }
}


/* Add a bullet: */

void add_bullet(int x, int y, int dir, int xm)
{
  int i, found;

  found = -1;

  for (i = 0; i < NUM_BULLETS && found == -1; i++)
    {
      if (!bullets[i].alive)
        found = i;
    }

  if (found != -1)
    {
      bullets[found].alive = YES;

      if (dir == RIGHT)
        {
          bullets[found].x = x + 32;
          bullets[found].xm = BULLET_XM + xm;
        }
      else
        {
          bullets[found].x = x;
          bullets[found].xm = -BULLET_XM + xm;
        }

      bullets[found].y = y;
      bullets[found].ym = BULLET_STARTING_YM;

      play_sound(sounds[SND_SHOOT]);
    }
}


void drawendscreen(void)
{
  char str[80];

  clearscreen(0, 0, 0);

  drawcenteredtext("GAMEOVER", 200, letters_red, NO_UPDATE);

  sprintf(str, "SCORE: %d", score);
  drawcenteredtext(str, 224, letters_gold, NO_UPDATE);

  sprintf(str, "DISTROS: %d", distros);
  drawcenteredtext(str, 256, letters_blue, NO_UPDATE);

  SDL_Flip(screen);
  SDL_Delay(2000);
}

void drawresultscreen(void)
{
  char str[80];

  clearscreen(0, 0, 0);

  drawcenteredtext("Result:", 200, letters_red, NO_UPDATE);

  sprintf(str, "SCORE: %d", score);
  drawcenteredtext(str, 224, letters_gold, NO_UPDATE);

  sprintf(str, "DISTROS: %d", distros);
  drawcenteredtext(str, 256, letters_blue, NO_UPDATE);

  SDL_Flip(screen);
  /*SDL_Delay(2000);*/
  sleep(2);
}


