//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Michael George <mike@georgetech.com>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <stdlib.h>
#include <string>
#include "configfile.h"
#include "setup.h"
#include "globals.h"
#include "lispreader.h"
#include "player.h"

#ifdef WIN32
const char * config_filename = "/st_config.dat";
#else
const char * config_filename = "/config";
#endif

static void defaults ()
{
  /* Set defaults: */
  debug_mode = false;

  use_fullscreen = true;
  show_fps = false;
  use_gl = false;

#ifndef NOSOUND
  audio_device = true;
  use_sound = true;
  use_music = true;
#else
  bool audio_device = false;
  bool use_sound = false;
  bool use_music = false;
#endif
}

void loadconfig(void)
{
  FILE * file = NULL;

  defaults();

  /* override defaults from config file */

  file = opendata(config_filename, "r");

  if (file == NULL)
    return;

  /* read config file */

  lisp_stream_t   stream;
  lisp_object_t * root_obj = NULL;

  lisp_stream_init_file (&stream, file);
  root_obj = lisp_read (&stream);

  if (root_obj->type == LISP_TYPE_EOF || root_obj->type == LISP_TYPE_PARSE_ERROR)
    return;

  if (strcmp(lisp_symbol(lisp_car(root_obj)), "supertux-config") != 0)
    return;

  LispReader reader(lisp_cdr(root_obj));

  reader.read_bool("fullscreen", &use_fullscreen);
#ifndef NOSOUND
  reader.read_bool("sound",      &use_sound);
  reader.read_bool("music",      &use_music);
#endif
  reader.read_bool("show_fps",   &show_fps);

  std::string video;
  reader.read_string ("video", &video);
  if (video == "opengl")
    use_gl = true;
  else
    use_gl = false;

  reader.read_int ("joystick", &joystick_num);
  if (!(joystick_num >= 0))
    use_joystick = false;
  else
    use_joystick = true;

#ifndef GP2X
  reader.read_int ("joystick-x", &joystick_keymap.x_axis);
  reader.read_int ("joystick-y", &joystick_keymap.y_axis);
  reader.read_int ("joystick-a", &joystick_keymap.a_button);
  reader.read_int ("joystick-b", &joystick_keymap.b_button);
  reader.read_int ("joystick-start", &joystick_keymap.start_button);
  reader.read_int ("joystick-deadzone", &joystick_keymap.dead_zone);
#else
  reader.read_int ("joystick-up", &joystick_keymap.up_button);
  reader.read_int ("joystick-down", &joystick_keymap.down_button);
  reader.read_int ("joystick-right", &joystick_keymap.right_button);
  reader.read_int ("joystick-left", &joystick_keymap.left_button);
  reader.read_int ("joystick-a", &joystick_keymap.a_button);
  reader.read_int ("joystick-b", &joystick_keymap.b_button);
  reader.read_int ("joystick-start", &joystick_keymap.start_button);
#endif

  reader.read_int ("keyboard-jump", &keymap.jump);
  reader.read_int ("keyboard-duck", &keymap.duck);
  reader.read_int ("keyboard-left", &keymap.left);
  reader.read_int ("keyboard-right", &keymap.right);
  reader.read_int ("keyboard-fire", &keymap.fire);

  lisp_free(root_obj);
  fclose(file);
}

void saveconfig (void)
{
  /* write settings to config file */

  FILE * config = opendata(config_filename, "w");

  if(config)
    {
      fprintf(config, "(supertux-config\n");
      fprintf(config, "\t;; the following options can be set to #t or #f:\n");
      fprintf(config, "\t(fullscreen %s)\n", use_fullscreen ? "#t" : "#f");
#ifndef NOSOUND
	  fprintf(config, "\t(sound      %s)\n", use_sound      ? "#t" : "#f");
      fprintf(config, "\t(music      %s)\n", use_music      ? "#t" : "#f");
#else
	  fprintf(config, "\t(sound      %s)\n", "#f");
      fprintf(config, "\t(music      %s)\n", "#f");
#endif
      fprintf(config, "\t(show_fps   %s)\n", show_fps       ? "#t" : "#f");

      fprintf(config, "\n\t;; either \"opengl\" or \"sdl\"\n");
      fprintf(config, "\t(video      \"%s\")\n", use_gl ? "opengl" : "sdl");

      fprintf(config, "\n\t;; joystick number (-1 means no joystick):\n");
      fprintf(config, "\t(joystick   %d)\n", use_joystick ? joystick_num : -1);

#ifndef GP2X
      fprintf(config, "\t(joystick-x   %d)\n", joystick_keymap.x_axis);
      fprintf(config, "\t(joystick-y   %d)\n", joystick_keymap.y_axis);
      fprintf(config, "\t(joystick-a   %d)\n", joystick_keymap.a_button);
      fprintf(config, "\t(joystick-b   %d)\n", joystick_keymap.b_button);
      fprintf(config, "\t(joystick-start  %d)\n", joystick_keymap.start_button);
      fprintf(config, "\t(joystick-deadzone  %d)\n", joystick_keymap.dead_zone);
#else
      fprintf(config, "\t(joystick-up   %d)\n", joystick_keymap.up_button);
      fprintf(config, "\t(joystick-down   %d)\n", joystick_keymap.down_button);
      fprintf(config, "\t(joystick-right   %d)\n", joystick_keymap.right_button);
      fprintf(config, "\t(joystick-left   %d)\n", joystick_keymap.left_button);
      fprintf(config, "\t(joystick-a   %d)\n", joystick_keymap.a_button);
      fprintf(config, "\t(joystick-b   %d)\n", joystick_keymap.b_button);
      fprintf(config, "\t(joystick-start  %d)\n", joystick_keymap.start_button);
#endif

      fprintf(config, "\t(keyboard-jump  %d)\n", keymap.jump);
      fprintf(config, "\t(keyboard-duck  %d)\n", keymap.duck);
      fprintf(config, "\t(keyboard-left  %d)\n", keymap.left);
      fprintf(config, "\t(keyboard-right %d)\n", keymap.right);
      fprintf(config, "\t(keyboard-fire  %d)\n", keymap.fire);

      fprintf(config, ")\n");

      fclose(config);
    }

}

/* EOF */
