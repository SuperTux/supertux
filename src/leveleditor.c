
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* leveleditor.c - A built-in level editor for SuperTux
 by Ricardo Cruz <rick2@aeiou.pt>                      */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_image.h>

#include "leveleditor.h"
#include "gameloop.h"
#include "screen.h"
#include "defines.h"
#include "globals.h"
#include "setup.h"
#include "menu.h"

#include "badguy.h"

char levelname[60];
char leveltheme[60];
bad_guy_type bad_guys[NUM_BAD_GUYS];
SDL_Surface *img_bsod_left[4], *img_laptop_left[3], *img_money_left[2];
char song_title[60];
char levelfilename[100];

int time_left;
unsigned char* tiles[15];
int bkgd_red, bkgd_green, bkgd_blue, level_width;

SDL_Surface *selection;

/* definitions to aid development */
#define DONE_LEVELEDITOR    1
#define DONE_QUIT        2
#define DONE_CHANGELEVEL  3

/* definitions that affect gameplay */
#define KEY_CURSOR_SPEED 32
#define KEY_CURSOR_FASTSPEED 64
#define KEY_LEFT_MARGIN 160
#define KEY_RIGHT_MARGIN 480

#define MOUSE_LEFT_MARGIN 32
#define MOUSE_RIGHT_MARGIN 608
#define MOUSE_POS_SPEED 32

/* gameloop funcs declerations */

void loadlevelgfx(void);
void unloadlevelgfx(void);
void add_bad_guy(int x, int y, int kind);
void loadshared(void);
void unloadshared(void);
void drawshape(int x, int y, unsigned char c);

/* own declerations */

void savelevel();
void le_loadlevel(void);
void le_change(int x, int y, int sx, unsigned char c);
void showhelp();

/* FIXME: Needs to be implemented. It should ask the user for the level(file)name and then let him create a new level based on this. */
void newlevel()
{

}

/* FIXME: It should let select the user a level, which is in the leveldirectory and then load it. */
void selectlevel()
{

}

int leveleditor()
{
char str[10];


strcpy(levelfilename,"level1");

initmenu();
menumenu = MENU_LEVELEDITOR;
show_menu = YES;

loadshared();
le_loadlevel();
loadlevelgfx();

selection = load_image(DATA_PREFIX "/images/leveleditor/select.png", USE_ALPHA);

int done;
done = 0;

int x, y, i;	/* for cicles */
int pos_x, cursor_x, cursor_y, old_cursor_x, fire;

pos_x = 0;
cursor_x = 3*32;
old_cursor_x = cursor_x;
cursor_y = 2*32;
fire = DOWN;

SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

SDL_Event event;
SDLKey key;
SDLMod keymod;

while(1)
	{
	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, bkgd_red, bkgd_green, bkgd_blue));

	keymod = SDL_GetModState();

	while(SDL_PollEvent(&event))
		{
		// testing SDL_KEYDOWN, SDL_KEYUP and SDL_QUIT events
		switch(event.type)
			{
			case SDL_KEYDOWN:	// key pressed
				key = event.key.keysym.sym;
				if(show_menu)
				{
				menu_event(key);
				break;
				}
				switch(key)
					{
					case SDLK_LEFT:
						if(fire == DOWN)
							cursor_x -= KEY_CURSOR_SPEED;
						else
							cursor_x -= KEY_CURSOR_FASTSPEED;

						if(cursor_x < 0)
							cursor_x = 0;
						break;
					case SDLK_RIGHT:
						if(fire == DOWN)
							cursor_x += KEY_CURSOR_SPEED;
						else
							cursor_x += KEY_CURSOR_FASTSPEED;

						if(cursor_x > (level_width*32) - 1)
							cursor_x = (level_width*32) - 1;
						break;
					case SDLK_UP:
						if(fire == DOWN)
							cursor_y -= KEY_CURSOR_SPEED;
						else
							cursor_y -= KEY_CURSOR_FASTSPEED;

						if(cursor_y < 0)
							cursor_y = 0;
						break;
					case SDLK_DOWN:
						if(fire == DOWN)
							cursor_y += KEY_CURSOR_SPEED;
						else
							cursor_y += KEY_CURSOR_FASTSPEED;

						if(cursor_y > 480-32)
							cursor_y = 480-32;
						break;
					case SDLK_LCTRL:
						fire = UP;
						break;
					case SDLK_F1:
						showhelp();
						break;
					case SDLK_HOME:
						cursor_x = 0;
						break;
					case SDLK_END:
						cursor_x = (level_width * 32) - 32;
						break;
					case SDLK_PERIOD:
						le_change(cursor_x, cursor_y, 0, '.');
						break;
					case SDLK_a:
						if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
							le_change(cursor_x, cursor_y, 0, 'A');
						else
							le_change(cursor_x, cursor_y, 0, 'a');
						break;
					case SDLK_b:
						if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
							le_change(cursor_x, cursor_y, 0, 'B');
						break;
					case SDLK_c:
						if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
							le_change(cursor_x, cursor_y, 0, 'C');
						else
							le_change(cursor_x, cursor_y, 0, 'c');
						break;
					case SDLK_d:
						if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
							le_change(cursor_x, cursor_y, 0, 'D');
						else
							le_change(cursor_x, cursor_y, 0, 'd');
						break;
					case SDLK_e:
						if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
							le_change(cursor_x, cursor_y, 0, 'E');
						else
							le_change(cursor_x, cursor_y, 0, 'e');
						break;
					case SDLK_f:
						if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
							le_change(cursor_x, cursor_y, 0, 'F');
						else
							le_change(cursor_x, cursor_y, 0, 'f');
						break;
					case SDLK_g:
						if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
							le_change(cursor_x, cursor_y, 0, 'G');
						else
							le_change(cursor_x, cursor_y, 0, 'g');
						break;
					case SDLK_h:
						if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
							le_change(cursor_x, cursor_y, 0, 'H');
						else
							le_change(cursor_x, cursor_y, 0, 'h');
						break;
					case SDLK_i:
						if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
							le_change(cursor_x, cursor_y, 0, 'I');
						else
							le_change(cursor_x, cursor_y, 0, 'i');
						break;
					case SDLK_j:
						if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
							le_change(cursor_x, cursor_y, 0, 'J');
						else
							le_change(cursor_x, cursor_y, 0, 'j');
						break;
					case SDLK_x:
						if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
							le_change(cursor_x, cursor_y, 0, 'X');
						else
							le_change(cursor_x, cursor_y, 0, 'x');
						break;
					case SDLK_y:
						if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
							le_change(cursor_x, cursor_y, 0, 'Y');
						else
							le_change(cursor_x, cursor_y, 0, 'y');
						break;
					case SDLK_LEFTBRACKET:
						le_change(cursor_x, cursor_y, 0, '[');
						break;
					case SDLK_RIGHTBRACKET:
						le_change(cursor_x, cursor_y, 0, ']');
						break;
 					case SDLK_HASH:
					case SDLK_3:
						if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
							le_change(cursor_x, cursor_y, 0, '#');
						break;
 					case SDLK_DOLLAR:
					case SDLK_4:
						if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
							le_change(cursor_x, cursor_y, 0, '$');
						break;
					case SDLK_BACKSLASH:
						if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
							le_change(cursor_x, cursor_y, 0, '|');
						else
							le_change(cursor_x, cursor_y, 0, '\\');
						break;
 					case SDLK_CARET:
						le_change(cursor_x, cursor_y, 0, '^');
						break;
 					case SDLK_AMPERSAND:
					case SDLK_6:
						if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
							le_change(cursor_x, cursor_y, 0, '&');
						break;
 					case SDLK_EQUALS:
					case SDLK_0:
						if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
							le_change(cursor_x, cursor_y, 0, '=');
						else		/* let's add a bad guy */
							le_change(cursor_x, cursor_y, 0, '0');

							for(i = 0; i < NUM_BAD_GUYS; ++i)
								if (bad_guys[i].alive == NO)
									{
									bad_guys[i].alive = YES;
									bad_guys[i].kind = BAD_BSOD;
									bad_guys[i].x = (((int)cursor_x/32)*32);
									bad_guys[i].y = (((int)cursor_y/32)*32);
									i = NUM_BAD_GUYS;
									}
						break;
					case SDLK_1:
						if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
							le_change(cursor_x, cursor_y, 0, '!');
						else		/* let's add a bad guy */
							le_change(cursor_x, cursor_y, 0, '1');

							for(i = 0; i < NUM_BAD_GUYS; ++i)
								if (bad_guys[i].alive == NO)
									{
									bad_guys[i].alive = YES;
									bad_guys[i].kind = BAD_LAPTOP;
									bad_guys[i].x = (((int)cursor_x/32)*32);
									bad_guys[i].y = (((int)cursor_y/32)*32);
									i = NUM_BAD_GUYS;
									}
						break;
					case SDLK_2:
						le_change(cursor_x, cursor_y, 0, '2');

						for(i = 0; i < NUM_BAD_GUYS; ++i)
							if (bad_guys[i].alive == NO)
								{
								bad_guys[i].alive = YES;
								bad_guys[i].kind = BAD_MONEY;
								bad_guys[i].x = (((int)cursor_x/32)*32);
								bad_guys[i].y = (((int)cursor_y/32)*32);
								i = NUM_BAD_GUYS;
								}
						break;
					case SDLK_PLUS:
						if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
							le_change(cursor_x, cursor_y, 0, '*');
						break;
					default:
						break;
					}
				break;
			case SDL_KEYUP:	// key released
				switch(event.key.keysym.sym)
					{
					case SDLK_LCTRL:
						fire = DOWN;
						break;
					case SDLK_ESCAPE:
						if(!show_menu)
						show_menu = YES;
						else
						show_menu = NO;
						break;
					default:
						break;
					}
				break;
			case SDL_MOUSEBUTTONDOWN:
				if(event.button.button == SDL_BUTTON_LEFT)
					{
					x = event.motion.x;
					y = event.motion.y;

					cursor_x = ((int)(pos_x + x) / 32) * 32;
					cursor_y = ((int) y / 32) * 32;
					}
				break;
			case SDL_MOUSEMOTION:
					x = event.motion.x;
					y = event.motion.y;

					cursor_x = ((int)(pos_x + x) / 32) * 32;
					cursor_y = ((int) y / 32) * 32;
					break;
			case SDL_QUIT:	// window closed
				done = DONE_QUIT;
				break;
			default:
				break;
			}
		}

	/* mouse movements */
	x = event.motion.x;
	if(x < MOUSE_LEFT_MARGIN)
		pos_x -= MOUSE_POS_SPEED;
	else if(x > MOUSE_RIGHT_MARGIN)
		pos_x += MOUSE_POS_SPEED;

 if(old_cursor_x != cursor_x)
		{
		if(cursor_x < pos_x + KEY_LEFT_MARGIN)
			pos_x = cursor_x - KEY_LEFT_MARGIN;

		if(cursor_x > pos_x + KEY_RIGHT_MARGIN)
			pos_x = cursor_x - KEY_RIGHT_MARGIN;
		}

	if(pos_x < 0)
		pos_x = 0;
	if(pos_x > (level_width * 32) - 640)
		pos_x = (level_width * 32) - 640;

	old_cursor_x = cursor_x;

	for (y = 0; y < 15; ++y)
		for (x = 0; x < 21; ++x)
			drawshape(x * 32, y * 32, tiles[y][x + (pos_x / 32)]);

	/* Draw the Bad guys: */
	for (i = 0; i < NUM_BAD_GUYS; ++i)
		{
/* printf("\nbad_guys[%i].alive = %i", i, bad_guys[i].alive); */
		if(bad_guys[i].alive == NO)
			continue;
/* to support frames: img_bsod_left[(frame / 5) % 4] */
		if(bad_guys[i].kind == BAD_BSOD)
			drawimage(img_bsod_left[0], ((int)(bad_guys[i].x - pos_x)/32)*32, bad_guys[i].y, NO_UPDATE);
		else if(bad_guys[i].kind == BAD_LAPTOP)
			drawimage(img_laptop_left[0], ((int)(bad_guys[i].x - pos_x)/32)*32, bad_guys[i].y, NO_UPDATE);
		else if (bad_guys[i].kind == BAD_MONEY)
			drawimage(img_money_left[0], ((int)(bad_guys[i].x - pos_x)/32)*32, bad_guys[i].y, NO_UPDATE);
		}

		
	drawimage(selection, ((int)(cursor_x - pos_x)/32)*32, cursor_y, NO_UPDATE);

	sprintf(str, "%d", time_left);
	drawtext("TIME", 324, 0, letters_blue, NO_UPDATE);
	drawtext(str, 404, 0, letters_gold, NO_UPDATE);
	
	sprintf(str, "%s", levelname);
	drawtext("NAME", 0, 0, letters_blue, NO_UPDATE);
	drawtext(str, 80, 0, letters_gold, NO_UPDATE);

	drawtext("F1 for Help", 10, 430, letters_blue, NO_UPDATE);

	if(show_menu)
	{
	done = drawmenu();
	if(done)
	return 0;
	}
	if(done == DONE_QUIT)
	return 1;
	
	SDL_Delay(50);
	SDL_Flip(screen);
	}

unloadlevelgfx();
unloadshared();

SDL_FreeSurface(selection);

/*if(done == DONE_SAVE)*/	/* let's save the changes	*/
/*	savelevel();*/
/*
if(done == DONE_CHANGELEVEL)		 change level 
	return leveleditor(level);
*/
return done;
}

void le_change(int x, int y, int sx, unsigned char c)
{
int xx, yy;

yy = (y / 32);
xx = ((x + sx) / 32);

/* if there is a bad guy over there, remove it */
int i;
for(i = 0; i < NUM_BAD_GUYS; ++i)
	if (bad_guys[i].alive)
		if(xx == bad_guys[i].x/32 && yy == bad_guys[i].y/32)
			bad_guys[i].alive = NO;


if (yy >= 0 && yy <= 15 && xx >= 0 && xx <= level_width)
	tiles[yy][xx] = c;
}

/* Save data for this level: */
void savelevel(void)
{
FILE * fi;
char * filename;

char str[80];

/* Save data file: */

filename = (char *) malloc(sizeof(char) * (strlen(DATA_PREFIX) + 20));
sprintf(filename, "%s/levels/%s.dat", DATA_PREFIX, levelfilename);
fi = fopen(filename, "w");
if (fi == NULL)
	{
	perror(filename);
	st_shutdown();
	free(filename);
	exit(-1);
	}
free(filename);


/* sptrinf("# Level created by SuperTux built-in editor", fi); */

fputs(levelname, fi);
fputs("\n", fi);
fputs(leveltheme, fi);
fputs("\n", fi);
sprintf(str, "%d\n", time_left);	/* time */
fputs(str, fi);
fputs(song_title, fi);	/* song filename */
sprintf(str, "%d\n", bkgd_red);	/* red background color */
fputs(str, fi);
sprintf(str, "%d\n", bkgd_green);	/* green background color */
fputs(str, fi);
sprintf(str, "%d\n", bkgd_blue);	/* blue background color */
fputs(str, fi);
sprintf(str, "%d\n", level_width);	/* level width */
fputs(str, fi);

int y;
for(y = 0; y < 15; ++y)
	{
	fputs(tiles[y], fi);
	fputs("\n", fi);
	}

fclose(fi);

drawcenteredtext("SAVED!", 240, letters_gold, NO_UPDATE);
SDL_Flip(screen);
SDL_Delay(1000);
}


/* I had to copy loadlevel, because loadlevel changes the tiles.
 For intance, badguys are removed from the tiles and we have to
 keep them there.                                             */

/* Load data for this level: */

void le_loadlevel(void)
{
FILE * fi;
char * filename;

char str[80];
char* line = malloc(sizeof(char)*10);/*[LEVEL_WIDTH + 5];*/

/* Load data file: */

filename = (char *) malloc(sizeof(char) * (strlen(DATA_PREFIX) + 20));
sprintf(filename, "%s/levels/%s.dat", DATA_PREFIX, levelfilename);
fi = fopen(filename, "r");
if (fi == NULL)
	{
	perror(filename);
	st_shutdown();
	free(filename);
	exit(-1);
	}
free(filename);

/* (Level title) */
fgets(str, 20, fi);
strcpy(levelname, str);
levelname[strlen(levelname)-1] = '\0';

/* (Level theme) */
fgets(str, 20, fi);
strcpy(leveltheme, str);
leveltheme[strlen(leveltheme)-1] = '\0';

fgets(line, 10, fi);
time_left = atoi(line);
fgets(str, 60, fi);
song_title[0]='\0';
strcpy(song_title,str);
fgets(line, 10, fi);
bkgd_red = atoi(line);
fgets(line, 10, fi);
bkgd_green= atoi(line);
fgets(line, 10, fi);
bkgd_blue = atoi(line);
fgets(line, 10, fi);
level_width = atoi(line);

free(line);
line = malloc(level_width*sizeof(char)+5);

int x, y;
for (y = 0; y < 15; ++y)
	{
	fgets(line, level_width + 5, fi);
	line[strlen(line) - 1] = '\0';
	free(tiles[y]);
	tiles[y] = malloc((strlen(line)+5)*sizeof(char));
	strcpy(tiles[y], line);
	}

fclose(fi);

/* Activate bad guys: */

/* as oposed to the gameloop.c func, this one doesn't remove
the badguys from tiles                                    */

for (y = 0; y < 15; ++y)
	for (x = 0; x < level_width; ++x)
		if (tiles[y][x] >= '0' && tiles[y][x] <= '9')
			add_bad_guy(x * 32, y * 32, tiles[y][x] - '0');


/* Set defaults: */

if(time_left == 0)
	time_left = 255;


/* Level Intro: */

clearscreen(0, 0, 0);

sprintf(str, "Editing Level %s", levelfilename);
drawcenteredtext(str, 200, letters_red, NO_UPDATE);

sprintf(str, "%s", levelname);
drawcenteredtext(str, 224, letters_gold, NO_UPDATE);

SDL_Flip(screen);

SDL_Delay(1000);
}

void showhelp()
{
drawcenteredtext("- Help -", 30, letters_red, NO_UPDATE);
drawtext("Keys:", 80, 60, letters_gold, NO_UPDATE);

char *text[] = {
	"X/x - Brick0",
	"Y/y - Brick1",
	"A/B/! - Box full",
	"a - Box empty",
	"C-F - Cloud0",
	"c-f - Cloud1",
	"G-J - Bkgd0",
	"g-j - Bkgd1",
	"# - Solid0",
	"[ - Solid1",
	"= - Solid2",
	"] - Solid3",
	"$ - Distro",
	"^ - Waves",
	"* - Poletop",
	"| - Pole",
	"\\ - Flag",
	"& - Water",
	"0-2 - BadGuys",
	"./Del - Remove tile",
	"Esc - Menu"};

int i;
for(i = 0; i < sizeof(text)/sizeof(char *); i++)
	drawtext(text[i], 40, 90+(i*16), letters_blue, NO_UPDATE);

SDL_Flip(screen);

int done;
done = 0;
SDL_Event event;

while(done == 0)
	while(SDL_PollEvent(&event))
		switch(event.type)
			{
			case SDL_KEYDOWN:		// key pressed
				done = 1;
				break;
			default:
				break;
			}
}
