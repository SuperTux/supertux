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

#include <string.h>

#include "defines.h"
#include "globals.h"
#include "menu.h"
#include "screen.h"
#include "setup.h"
#include "sound.h"
#include "scene.h"
#include "leveleditor.h"
#include "gameloop.h"
#include "timer.h"
#include "high_scores.h"

/* Set defaults */
void initmenu(void)
{
  menu_change = NO;
  show_menu = NO;
  menuitem = 0;
  menumenu = 0;
  menuaction = -1;

  delete_character = 0;
  strcpy(input_string, "");
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
	    text_drawf(&red_text, "Sound ON", 0, 224, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
            }
          else
            {
	    text_drawf(&red_text, "Sound OFF", 0, 224, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
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
	    text_drawf(&blue_text, "Sound ON", 0, 224, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
          else
	    text_drawf(&blue_text, "Sound OFF", 0, 224, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
        }
    }
  else
    { /* if audio_device != YES */
      /* let the user move over the deactivated option */
      if (menuitem == 1)
        {
	  text_drawf(&red_text, "Sound OFF", 0, 224, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
        }
      else
        {
	  text_drawf(&red_text, "Sound OFF", 0, 224, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
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
	      text_drawf(&red_text, "Music ON", 0, 256, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
            }
          else
            {
	      text_drawf(&red_text, "Music OFF", 0, 256, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
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
	      text_drawf(&blue_text, "Music ON", 0, 256, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
            }
          else
            {
	      text_drawf(&blue_text, "Music OFF", 0, 256, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
            }
        }
    }
  else
    { /* if audio_device != YES */
      /* let the user move over the deactivated option */
      if (menuitem == 2)
        {
	  text_drawf(&red_text, "Music OFF", 0, 256, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
        }
      else
        {
	  text_drawf(&black_text, "Music OFF", 0, 256, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
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
      text_drawf(&red_text, "Start Game", 0, 192, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
      if(menuaction == MN_HIT) /* we are ready to start the game, if this item got hit */
        {
          game_started = 1;
          show_menu = 0;
        }
    }
  else
    text_drawf(&blue_text, "Start Game", 0, 192, A_HMIDDLE, A_TOP, 2, NO_UPDATE);

  if(menuitem == 1)
    {
          text_drawf(&red_text, "Options", 0, 224, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
      if(menuaction == MN_HIT) /* Switch to the 'Options' menu */
        {
          menumenu = MENU_OPTIONS;
          menu_change = YES;
        }
    }
  else
        text_drawf(&blue_text, "Options", 0, 224, A_HMIDDLE, A_TOP, 2, NO_UPDATE);

  if(menuitem == 2)
    {
          text_drawf(&red_text, "Level editor", 0, 256, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
      if(menuaction == MN_HIT) /* Set variables, so that the level editor is executed */
        {
          level_editor_started = YES;
          show_menu = 0;
        }
    }
  else
          text_drawf(&blue_text, "Level editor", 0, 256, A_HMIDDLE, A_TOP, 2, NO_UPDATE);

  if(menuitem == 3)
    {
          text_drawf(&red_text, "Quit", 0, 288, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
      if(menuaction == MN_HIT) /* Quit a running game or the application */
        {
          return 1;
        }
    }
  else
    {
              text_drawf(&blue_text, "Quit", 0, 288, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
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
          text_drawf(&red_text, "Return To Game", 0, 192, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
      if(menuaction == MN_HIT) /* Don't show the menu anymore, if this item got hit */
        {
          show_menu = 0;
          st_pause_ticks_stop();
        }
    }
  else
          text_drawf(&blue_text, "Return To Game", 0, 192, A_HMIDDLE, A_TOP, 2, NO_UPDATE);

  if(menuitem == 1)
    {
          text_drawf(&red_text, "Save Game", 0, 224, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
      if(menuaction == MN_HIT) /* Don't show the menu anymore, if this item got hit */
        {
          show_menu = 0;
          savegame();
        }
    }
  else
  text_drawf(&blue_text, "Save Game", 0, 224, A_HMIDDLE, A_TOP, 2, NO_UPDATE);

  if(menuitem == 2)
    {
    text_drawf(&red_text, "Load Game", 0, 256, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
      if(menuaction == MN_HIT) /* Don't show the menu anymore, if this item got hit */
        {
          char *x = NULL;  /* In C, you can do this... */

          show_menu = 0;
          loadgame(x);
        }
    }
  else
      text_drawf(&blue_text, "Load Game", 0, 256, A_HMIDDLE, A_TOP, 2, NO_UPDATE);

  if(menuitem == 3)
    {
        text_drawf(&red_text, "Options", 0, 288, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
      if(menuaction == MN_HIT) /* Switch to the 'Options' menu */
        {
          menumenu = MENU_OPTIONS;
          menu_change = YES;
        }
    }
  else
          text_drawf(&blue_text, "Options", 0, 288, A_HMIDDLE, A_TOP, 2, NO_UPDATE);

  if(menuitem == 4)
    {
            text_drawf(&red_text, "Quit Game", 0, 320, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
      if(menuaction == MN_HIT) /* Quit a running game */
        return 1;
    }
  else
    {
            text_drawf(&blue_text, "Quit Game", 0, 320, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
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
      text_drawf(&red_text, "Fullscreen ON", 0, 192, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
      else
      text_drawf(&red_text, "Fullscreen OFF", 0, 192, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
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
      text_drawf(&blue_text, "Fullscreen ON", 0, 192, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
      else
      text_drawf(&blue_text, "Fullscreen OFF", 0, 192, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
    }

  /* handle menu sound on/off option */
  menu_option_sound();

  /* handle menu music on/off option */
  menu_option_music();

  if(menuitem == 3)
    {
    text_drawf(&red_text, "Back", 0, 288, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
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
    text_drawf(&blue_text, "Back", 0, 288, A_HMIDDLE, A_TOP, 2, NO_UPDATE);

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
        text_drawf(&red_text, "Return To Level Editor", 0, 192, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
      if(menuaction == MN_HIT) /* Don't show the menu anymore, if this item got hit */
        show_menu = 0;
    }
  else
  text_drawf(&blue_text, "Return To Level Editor", 0, 192, A_HMIDDLE, A_TOP, 2, NO_UPDATE);

  if(menuitem == 1)
    {
    text_drawf(&red_text, "New Level", 0, 224, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
      if(menuaction == MN_HIT) /* Don't show the menu anymore, if this item got hit */
        {
          show_menu = 0;
          newlevel();
        }
    }
  else
  text_drawf(&blue_text, "New Level", 0, 224, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
  
  if(menuitem == 2)
    {
    text_drawf(&red_text, "Load Level", 0, 256, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
      if(menuaction == MN_HIT) /* Create a new Level and load it into the level-editor. */
        {
          show_menu = 0;
          selectlevel();
        }
    }
  else
  text_drawf(&blue_text, "Load Level", 0, 256, A_HMIDDLE, A_TOP, 2, NO_UPDATE);

  if(menuitem == 3)
    {
   text_drawf(&red_text, "Save Level", 0, 288, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
      if(menuaction == MN_HIT) /* Save the current level in the level-editor. */
        {
          show_menu = 0;
          savelevel(&current_level,"default",level);
	  text_drawf(&gold_text, "SAVED!", 0, 240, A_HMIDDLE, A_TOP, 1, NO_UPDATE);
          flipscreen();
          SDL_Delay(1000);
        }
    }
  else
  text_drawf(&blue_text, "Save Level", 0, 288, A_HMIDDLE, A_TOP, 2, NO_UPDATE);

  if(menuitem == 4)
    {
    text_drawf(&red_text, "Quit Level Editor", 0, 320, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
      if(menuaction == MN_HIT) /* Quit the level-editor. (to the main-menu) */
        {
          return 1;
        }
    }
  else
  text_drawf(&blue_text, "Quit Level Editor", 0, 320, A_HMIDDLE, A_TOP, 2, NO_UPDATE);

  return 0;
}

/* Menu HighScore (ask for player's name) */
int menu_highscore()
{
char str[60];

while(delete_character > 0)	/* remove charactes */
	{
	hs_name[strlen(hs_name)-1] = '\0';
	delete_character--;
	}

strcat(hs_name, input_string);

text_drawf(&red_text, "Congratulations", 0, 130, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
text_draw(&red_text, "Your score:", 30, 250, 1, NO_UPDATE);
sprintf(str, "%d", hs_score);
text_draw(&blue_text, str, 350, 250, 1, NO_UPDATE);

text_draw(&red_text, "Enter your name:", 30, 280, 1, NO_UPDATE);
text_draw(&blue_text, hs_name, 350, 280, 1, NO_UPDATE);

strcpy(input_string, "");

if(menuaction == MN_HIT)    /* name written */
  show_menu = 0;

return 0;
}

/* --- MENU --- */
/* Draw the menu and execute the (menu)events */
int drawmenu(void)
{
  int quit = 0;

  menu_change = NO;


  if(menuaction == MN_UP)
      --menuitem;   /* Go one menu-item up */
  else if(menuaction == MN_DOWN)
    ++menuitem;     /* Go one menu-item down */


  switch(menumenu)
  {
  case MENU_MAIN:
      quit = menu_main();
    break;
  case MENU_GAME:
      quit = menu_game();
    break;
  case MENU_OPTIONS:
      quit = menu_options();
    break;
  case MENU_LEVELEDITOR:
      quit = menu_leveleditor();
    break;
  case MENU_HIGHSCORE:
      quit = menu_highscore();
    break;
  default:
    break;
  }

  menuaction = -1;

  return quit;
}

/* Check for menu event */
void menu_event(SDL_keysym* keysym)
{
SDLKey key = keysym->sym;
SDLMod keymod;
char ch[2];

keymod = SDL_GetModState();

/* If the current unicode character is an ASCII character,
   assign it to ch. */
if ( (keysym->unicode & 0xFF80) == 0 ) {
  ch[0] = keysym->unicode & 0x7F;
  ch[1] = '\0';
}
else {
  /* An International Character. */
}

switch(key)
	{
	case SDLK_UP:		/* Menu Up */
		menuaction = MN_UP;
		menu_change = YES;
		break;
	case SDLK_DOWN:		/* Menu Down */
		menuaction = MN_DOWN;
		menu_change = YES;
		break;
	case SDLK_SPACE:		/* Menu Hit */
	case SDLK_RETURN:
		menuaction = MN_HIT;
		menu_change = YES;
		break;

	case SDLK_DELETE:
	case SDLK_BACKSPACE:
		delete_character++;
		break;
	default:
		if( key >= SDLK_0 && key <= SDLK_9)
		 strcat( input_string, /* (key - SDLK_0) */ ch);
		else if( key >= SDLK_a && SDLK_z )
		{
		 strcat( input_string, ch);
		}
		break;
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

