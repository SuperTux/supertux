/*
  menu.c
  
  Super Tux - Menu
  
  by Tobias Glaesser
  tobi.web@gmx.de
  http://www.newbreedsoftware.com/supertux/
  
  December 20, 2003
*/

#ifdef LINUX
#include <pwd.h>
#include <sys/types.h>
#include <ctype.h>
#endif

#include "defines.h"
#include "globals.h"
#include "menu.h"
#include "screen.h"
#include "setup.h"
#include "sound.h"


/* Set defaults */
void initmenu(void)
{
  menu_change = NO;
  show_menu = NO;
  menuitem = 0;
  menumenu = 0;
  menuaction = -1;
}

/* ---- Menu Options - Item Sound On/off ----*/
void menu_option_sound()
{
  if (audio_device == YES) {
    if(menuitem == 1) {
       if(use_sound == YES) {
         drawcenteredtext("Sound ON", 224, letters_red, NO_UPDATE);
       }
       else {
         drawcenteredtext("Sound OFF", 224, letters_red, NO_UPDATE);
       }

       if(menuaction == MN_HIT) { /* Disable/Enable sound */
         if(use_sound == YES) {
           use_sound = NO;
         }
         else {
           use_sound = YES;
         }
         menu_change = YES;
       }
    }
    else {
      if(use_sound == YES)
        drawcenteredtext("Sound ON", 224, letters_blue, NO_UPDATE);
      else
        drawcenteredtext("Sound OFF", 224, letters_blue, NO_UPDATE);
    }
  }
  else { /* if audio_device != YES */
    /* let the user move over the deactivated option */
    if (menuitem == 1) {
      drawcenteredtext("Sound OFF", 224, letters_red, NO_UPDATE);
    }
    else {
      drawcenteredtext("Sound OFF", 224, letters_black, NO_UPDATE);
    }
  }
}


/* ---- Menu Options - Item Music On/off ----*/
void menu_option_music()
{
  if (audio_device == YES) {
    if(menuitem == 2) {
      if(use_music == YES) {
        drawcenteredtext("Music ON", 256, letters_red, NO_UPDATE);
      }
     else {
        drawcenteredtext("Music OFF", 256, letters_red, NO_UPDATE);
      }
      if(menuaction == MN_HIT) { /* Disable/Enable music */
        if(use_music == YES) {  /* In the menu no music is played, so we have to check only use_music */
	     if(playing_music()) 
              halt_music();
             use_music = NO;
        }
        else {
          use_music = YES;
          if (!playing_music()) {
            switch (current_music) {
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
        }
        menu_change = YES;
      }
    } /* if menuitem != 2 : print normal blue font */
    else {
      if(use_music == YES) {
        drawcenteredtext("Music ON", 256, letters_blue, NO_UPDATE);
      }
      else {
        drawcenteredtext("Music OFF", 256, letters_blue, NO_UPDATE);
      }
    }
  }
  else { /* if audio_device != YES */
   /* let the user move over the deactivated option */
    if (menuitem == 2) {
      drawcenteredtext("Music OFF", 256, letters_red, NO_UPDATE);
    }
    else {
      drawcenteredtext("Music OFF", 256, letters_black, NO_UPDATE);
    }
  }
}

/* --- MENU --- */
/* Draw the menu and execute the (menu)events */
int drawmenu(void)
{
  int quit = 0;

  menu_change = NO;


  if(menuaction == MN_UP)
    {
      /* Go one menu-item up, if possible */
      if(menuitem > 0)
        --menuitem;
    }
  else if(menuaction == MN_DOWN)
    ++menuitem; /* Go one menu-item down */


  if(menumenu == MENU_MAIN)
    {
      /* Does the menu item exist? If not, we reset to the most down item */
      if(menuitem >= MENU_MAIN_ITEM_MAX)
        menuitem = MENU_MAIN_ITEM_MAX - 1;

      /*The menu looks different, when the game is started */
      if(game_started)
        {
          if(menuitem == 0)
            {
              drawcenteredtext("Return To Game", 192, letters_red, NO_UPDATE);
              if(menuaction == MN_HIT) /* Don't show the menu anymore, if this item got hit */
                show_menu = 0;
            }
          else
            drawcenteredtext("Return To Game", 192, letters_blue, NO_UPDATE);
        }
      else
        {
          if(menuitem == 0)
            {
              drawcenteredtext("Start Game", 192, letters_red, NO_UPDATE);
              if(menuaction == MN_HIT) /* we are ready to start the game, if this item got hit */
                {
                  game_started = 1;
                  show_menu = 0;
                }
            }
          else
            drawcenteredtext("Start Game", 192, letters_blue, NO_UPDATE);
        }

      if(menuitem == 1)
        {
          drawcenteredtext("Options", 224, letters_red, NO_UPDATE);
          if(menuaction == MN_HIT) /* Switch to the 'Options' menu */
            {
              menumenu = MENU_OPTIONS;
              menu_change = YES;
            }
        }
      else
        drawcenteredtext("Options", 224, letters_blue, NO_UPDATE);

      if(menuitem == 2)
        {
          if(game_started)
            drawcenteredtext("Quit Game", 256, letters_red, NO_UPDATE);
          else
            drawcenteredtext("Quit", 256, letters_red, NO_UPDATE);
          if(menuaction == MN_HIT) /* Quit a running game or the application */
            {
              quit = 1;
            }
        }
      else
        {
          if(game_started)
            drawcenteredtext("Quit Game", 256, letters_blue, NO_UPDATE);
          else
            drawcenteredtext("Quit", 256, letters_blue, NO_UPDATE);
        }

    }
  else if(menumenu == MENU_OPTIONS)
    {
      if(menuitem >= MENU_OPTIONS_ITEM_MAX )
        menuitem = MENU_OPTIONS_ITEM_MAX - 1;

      if(menuitem == 0)
        {
          if(use_fullscreen)
            drawcenteredtext("Fullscreen ON", 192, letters_red, NO_UPDATE);
          else
            drawcenteredtext("Fullscreen OFF", 192, letters_red, NO_UPDATE);
          if(menuaction == MN_HIT) /* Disable/Enable fullscreen */
            {
              if(use_fullscreen)
                use_fullscreen = 0;
              else
                use_fullscreen = 1;
              st_video_setup();
              menu_change = YES;
            }
        }
      else
        {
          if(use_fullscreen)
            drawcenteredtext("Fullscreen ON", 192, letters_blue, NO_UPDATE);
          else
            drawcenteredtext("Fullscreen OFF", 192, letters_blue, NO_UPDATE);
        }

      /* handle menu sound on/off option */
      menu_option_sound();     

      /* handle menu music on/off option */
      menu_option_music();
   
      if(menuitem == 3)
        {
          drawcenteredtext("Back", 288, letters_red, NO_UPDATE);
          if(menuaction == MN_HIT) /* Go back to main menu. */
            {
              menumenu = MENU_MAIN;
              menu_change = YES;
            }
        }
      else
        drawcenteredtext("Back", 288, letters_blue, NO_UPDATE);

    }

  menuaction = -1;

  SDL_Flip(screen);

  return quit;
}

/* Check for menu event */
void menu_event(SDLKey key)
{


  if (key == SDLK_UP)
    {
      /* Menu Up */

      menuaction = MN_UP;
      menu_change = YES;
    }
  else if (key == SDLK_DOWN)
    {
      /* Menu Down */

      menuaction = MN_DOWN;
      menu_change = YES;
    }
  else if (key == SDLK_SPACE || key == SDLK_RETURN)
    {
      /* Menu Hit */

      menuaction = MN_HIT;
      menu_change = YES;
    }

  /* FIXME: NO JOYSTICK SUPPORT */
  /*#ifdef JOY_YES
  else if (event.type == SDL_JOYBUTTONDOWN)
   {
      Joystick button: Continue: 
     
     done = 1;
   }
  #endif*/

}

