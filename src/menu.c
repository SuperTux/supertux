/*
  menu.c
  
  Super Tux - Menu
  
  by Tobias Glaesser
  tobi.web@gmx.de
  http://www.newbreedsoftware.com/supertux/
  
  December 20, 2003 - December 30, 2003
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
#include "leveleditor.h"
#include "gameloop.h"

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
  if (audio_device == YES)
    {
      if(menuitem == 1)
        {
          if(use_sound == YES)
            {
              drawcenteredtext("Sound ON", 224, letters_red, NO_UPDATE, 2);
            }
          else
            {
              drawcenteredtext("Sound OFF", 224, letters_red, NO_UPDATE, 2);
            }

          if(menuaction == MN_HIT)
            { /* Disable/Enable sound */
              if(use_sound == YES)
                {
                  use_sound = NO;
                }
              else
                {
                  use_sound = YES;
                }
              menu_change = YES;
            }
        }
      else
        {
          if(use_sound == YES)
            drawcenteredtext("Sound ON", 224, letters_blue, NO_UPDATE, 2);
          else
            drawcenteredtext("Sound OFF", 224, letters_blue, NO_UPDATE, 2);
        }
    }
  else
    { /* if audio_device != YES */
      /* let the user move over the deactivated option */
      if (menuitem == 1)
        {
          drawcenteredtext("Sound OFF", 224, letters_red, NO_UPDATE, 2);
        }
      else
        {
          drawcenteredtext("Sound OFF", 224, letters_black, NO_UPDATE, 2);
        }
    }
}


/* ---- Menu Options - Item Music On/off ----*/
void menu_option_music()
{
  if (audio_device == YES)
    {
      if(menuitem == 2)
        {
          if(use_music == YES)
            {
              drawcenteredtext("Music ON", 256, letters_red, NO_UPDATE, 2);
            }
          else
            {
              drawcenteredtext("Music OFF", 256, letters_red, NO_UPDATE, 2);
            }
          if(menuaction == MN_HIT)
            { /* Disable/Enable music */
              if(use_music == YES)
                {  /* In the menu no music is played, so we have to check only use_music */
                  if(playing_music())
                    halt_music();
                  use_music = NO;
                }
              else
                {
                  use_music = YES;
                  if (!playing_music())
                    {
		    play_current_music();
                    }
                }
              menu_change = YES;
            }
        } /* if menuitem != 2 : print normal blue font */
      else
        {
          if(use_music == YES)
            {
              drawcenteredtext("Music ON", 256, letters_blue, NO_UPDATE, 2);
            }
          else
            {
              drawcenteredtext("Music OFF", 256, letters_blue, NO_UPDATE, 2);
            }
        }
    }
  else
    { /* if audio_device != YES */
      /* let the user move over the deactivated option */
      if (menuitem == 2)
        {
          drawcenteredtext("Music OFF", 256, letters_red, NO_UPDATE, 2);
        }
      else
        {
          drawcenteredtext("Music OFF", 256, letters_black, NO_UPDATE, 2);
        }
    }
}

int menu_main(void)
{
  /* Does the menu item exist? If not, we reset to the most down item */
  if(menuitem > MENU_MAIN_ITEM_MAX)
    menuitem =0;
  else if(menuitem < 0)
    menuitem = MENU_MAIN_ITEM_MAX;

  /*The menu looks different, when the game is started */
  if(menuitem == 0)
    {
      drawcenteredtext("Start Game", 192, letters_red, NO_UPDATE, 2);
      if(menuaction == MN_HIT) /* we are ready to start the game, if this item got hit */
        {
          game_started = 1;
          show_menu = 0;
        }
    }
  else
    drawcenteredtext("Start Game", 192, letters_blue, NO_UPDATE, 2);

  if(menuitem == 1)
    {
      drawcenteredtext("Options", 224, letters_red, NO_UPDATE, 2);
      if(menuaction == MN_HIT) /* Switch to the 'Options' menu */
        {
          menumenu = MENU_OPTIONS;
          menu_change = YES;
        }
    }
  else
    drawcenteredtext("Options", 224, letters_blue, NO_UPDATE, 2);

  if(menuitem == 2)
    {
      drawcenteredtext("Level editor", 256, letters_red, NO_UPDATE, 2);
      if(menuaction == MN_HIT) /* Set variables, so that the level editor is executed */
        {
          level_editor_started = YES;
          show_menu = 0;
        }
    }
  else
    drawcenteredtext("Level editor", 256, letters_blue, NO_UPDATE, 2);

  if(menuitem == 3)
    {
      drawcenteredtext("Quit", 288, letters_red, NO_UPDATE, 2);
      if(menuaction == MN_HIT) /* Quit a running game or the application */
        {
          return 1;
        }
    }
  else
    {
      drawcenteredtext("Quit", 288, letters_blue, NO_UPDATE, 2);
    }

  return 0;
}

int menu_game(void)
{
  /* Does the menu item exist? If not, we reset to the most down item */
  if(menuitem > MENU_GAME_ITEM_MAX)
    menuitem = 0;
  else if(menuitem < 0)
    menuitem = MENU_GAME_ITEM_MAX;

  /*The menu looks different, when the game is started */
  if(menuitem == 0)
    {
      drawcenteredtext("Return To Game", 192, letters_red, NO_UPDATE, 2);
      if(menuaction == MN_HIT) /* Don't show the menu anymore, if this item got hit */
        show_menu = 0;
    }
  else
    drawcenteredtext("Return To Game", 192, letters_blue, NO_UPDATE, 2);
    
  if(menuitem == 1)
    {
      drawcenteredtext("Save Game", 224, letters_red, NO_UPDATE, 2);
      if(menuaction == MN_HIT) /* Don't show the menu anymore, if this item got hit */
      {
        show_menu = 0;
	savegame();
      }
    }
  else
    drawcenteredtext("Save Game", 224, letters_blue, NO_UPDATE, 2);
    
  if(menuitem == 2)
    {
      drawcenteredtext("Load Game", 256, letters_red, NO_UPDATE, 2);
      if(menuaction == MN_HIT) /* Don't show the menu anymore, if this item got hit */
      {
	char *x = NULL;  /* In C, you can do this... */

        show_menu = 0;
	loadgame(x);
      }
    }
  else
    drawcenteredtext("Load Game", 256, letters_blue, NO_UPDATE, 2);
    
  if(menuitem == 3)
    {
      drawcenteredtext("Options", 288, letters_red, NO_UPDATE, 2);
      if(menuaction == MN_HIT) /* Switch to the 'Options' menu */
        {
          menumenu = MENU_OPTIONS;
          menu_change = YES;
        }
    }
  else
    drawcenteredtext("Options", 288, letters_blue, NO_UPDATE, 2);

  if(menuitem == 4)
    {
      drawcenteredtext("Quit Game", 320, letters_red, NO_UPDATE, 2);
      if(menuaction == MN_HIT) /* Quit a running game */
        return 1;
    }
  else
    {
      drawcenteredtext("Quit Game", 320, letters_blue, NO_UPDATE, 2);
    }

  return 0;
}

int menu_options(void)
{
  if(menuitem > MENU_OPTIONS_ITEM_MAX )
    menuitem = 0;
  else if(menuitem < 0)
    menuitem = MENU_OPTIONS_ITEM_MAX;

  if(menuitem == 0)
    {
      if(use_fullscreen)
        drawcenteredtext("Fullscreen ON", 192, letters_red, NO_UPDATE, 2);
      else
        drawcenteredtext("Fullscreen OFF", 192, letters_red, NO_UPDATE, 2);
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
        drawcenteredtext("Fullscreen ON", 192, letters_blue, NO_UPDATE, 2);
      else
        drawcenteredtext("Fullscreen OFF", 192, letters_blue, NO_UPDATE, 2);
    }

  /* handle menu sound on/off option */
  menu_option_sound();

  /* handle menu music on/off option */
  menu_option_music();

  if(menuitem == 3)
    {
      drawcenteredtext("Back", 288, letters_red, NO_UPDATE, 2);
      if(menuaction == MN_HIT) /* Go back to main menu. */
        {
          if(game_started)
            menumenu = MENU_GAME;
          else
            menumenu = MENU_MAIN;
          menu_change = YES;
        }
    }
  else
    drawcenteredtext("Back", 288, letters_blue, NO_UPDATE, 2);

  return 0;
}

/* Menu LevelEditor */
int menu_leveleditor(void)
{
  if(menuitem > MENU_LEVELEDITOR_ITEM_MAX )
    menuitem = 0;
  else if(menuitem < 0)
    menuitem = MENU_LEVELEDITOR_ITEM_MAX;

  if(menuitem == 0)
    {
      drawcenteredtext("Return To Level Editor", 192, letters_red, NO_UPDATE, 2);
      if(menuaction == MN_HIT) /* Don't show the menu anymore, if this item got hit */
        show_menu = 0;
    }
  else
    drawcenteredtext("Return To Level Editor", 192, letters_blue, NO_UPDATE, 2);

  if(menuitem == 1)
    {
      drawcenteredtext("New Level", 224, letters_red, NO_UPDATE, 2);
      if(menuaction == MN_HIT) /* Don't show the menu anymore, if this item got hit */
        {
          show_menu = 0;
          newlevel();
        }
    }
  else
    drawcenteredtext("New Level", 224, letters_blue, NO_UPDATE, 2);
  if(menuitem == 2)
    {
      drawcenteredtext("Load Level", 256, letters_red, NO_UPDATE, 2);
      if(menuaction == MN_HIT) /* Create a new Level and load it into the level-editor. */
        {
          show_menu = 0;
          selectlevel();
        }
    }
  else
    drawcenteredtext("Load Level", 256, letters_blue, NO_UPDATE, 2);
  if(menuitem == 3)
    {
      drawcenteredtext("Save Level", 288, letters_red, NO_UPDATE, 2);
      if(menuaction == MN_HIT) /* Save the current level in the level-editor. */
        {
          show_menu = 0;
          savelevel();
        }
    }
  else
    drawcenteredtext("Save Level", 288, letters_blue, NO_UPDATE, 2);

  if(menuitem == 4)
    {
      drawcenteredtext("Quit Level Editor", 320, letters_red, NO_UPDATE, 2);
      if(menuaction == MN_HIT) /* Quit the level-editor. (to the main-menu) */
        {
          return 1;
        }
    }
  else
    drawcenteredtext("Quit Level Editor", 320, letters_blue, NO_UPDATE, 2);

  return 0;
}

/* --- MENU --- */
/* Draw the menu and execute the (menu)events */
int drawmenu(void)
{
  int quit = 0;

  menu_change = NO;


  if(menuaction == MN_UP)
    {
      /* Go one menu-item up */
      --menuitem;
    }
  else if(menuaction == MN_DOWN)
    ++menuitem; /* Go one menu-item down */


  if(menumenu == MENU_MAIN)
    {
      quit = menu_main();
    }
  else if(menumenu == MENU_GAME)
    {
      quit = menu_game();
    }
  else if(menumenu == MENU_OPTIONS)
    {
      quit = menu_options();
    }
  else if(menumenu == MENU_LEVELEDITOR)
    {
      quit = menu_leveleditor();
    }

  menuaction = -1;

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

