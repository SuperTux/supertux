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

#include <config.h>

#include <cstdlib>
#include <string>
#include <stdexcept>

#include "configfile.h"
#include "app/setup.h"
#include "app/globals.h"
#include "audio/sound_manager.h"
#include "lisp/parser.h"

using namespace SuperTux;

#ifdef WIN32
const char * config_filename = ("/"+ package_symbol_name + "_config.dat").c_str();
#else
const char * config_filename = "/config";
#endif

Config* SuperTux::config = 0;

static void defaults ()
{
  /* Set defaults: */
  debug_mode = false;
  SoundManager::get()->set_audio_device_available(true);

  use_fullscreen = false;
  show_fps = false;
  use_gl = false;

  SoundManager::get()->enable_sound(true);
  SoundManager::get()->enable_music(true);
}

FILE * SuperTux::opendata(const std::string& rel_filename, const char *mode)
{
  std::string filename;
  FILE * fi;

  filename = st_dir + rel_filename;

  /* Try opening the file: */
  fi = fopen(filename.c_str(), mode);

  if (fi == NULL)
    {
      fprintf(stderr, "Warning: Unable to open the file \"%s\" ", filename.c_str());

      if (strcmp(mode, "r") == 0)
        fprintf(stderr, "for read!!!\n");
      else if (strcmp(mode, "w") == 0)
        fprintf(stderr, "for write!!!\n");
    }

  return(fi);
}

void Config::load()
{
  defaults();

  lisp::Parser parser;
  try {
    std::auto_ptr<lisp::Lisp> root (parser.parse(st_dir + config_filename));

    const lisp::Lisp* config_lisp = root->get_lisp(
        package_symbol_name + "-config");
    if(!config_lisp)
      throw new std::runtime_error("Config file is not a supertux-config file");

    config_lisp->get("fullscreen", use_fullscreen);
    bool temp = false;
    if(config_lisp->get("sound", temp))
      SoundManager::get()->enable_sound(temp);
    if(config_lisp->get("music", temp))
      SoundManager::get()->enable_music(temp);
    config_lisp->get("show_fps",   show_fps);

    std::string video;
    if(config_lisp->get("video", video)) {
      if (video == "opengl")
        use_gl = true;
      else
        use_gl = false;
    }

    joystick_num = 0;
    config_lisp->get("joystick", joystick_num);
    
    if (joystick_num >= 0) {
      config_lisp->get("joystick-x", joystick_keymap.x_axis);
      config_lisp->get("joystick-y", joystick_keymap.y_axis);
      config_lisp->get("joystick-a", joystick_keymap.a_button);
      config_lisp->get("joystick-b", joystick_keymap.b_button);
      config_lisp->get("joystick-start", joystick_keymap.start_button);
      config_lisp->get("joystick-deadzone", joystick_keymap.dead_zone);
    }

    customload(config_lisp);
  } catch(std::exception& e) {
    std::cerr << "Couldn't load configfile: " << e.what() << "\n";
  }
}

void Config::save ()
{
  /* write settings to config file */
  FILE * config = opendata(config_filename, "w");

  if(config)
    {
      fprintf(config, ("("+package_symbol_name+"-config\n").c_str());
      fprintf(config, "\t;; the following options can be set to #t or #f:\n");
      fprintf(config, "\t(fullscreen %s)\n", use_fullscreen ? "#t" : "#f");
      fprintf(config, "\t(sound      %s)\n", SoundManager::get()->sound_enabled()      ? "#t" : "#f");
      fprintf(config, "\t(music      %s)\n", SoundManager::get()->music_enabled()      ? "#t" : "#f");
      fprintf(config, "\t(show_fps   %s)\n", show_fps       ? "#t" : "#f");

      fprintf(config, "\n\t;; either \"opengl\" or \"sdl\"\n");
      fprintf(config, "\t(video      \"%s\")\n", use_gl ? "opengl" : "sdl");

      if(use_joystick)
        {
        fprintf(config, "\n\t;; joystick number:\n");
        fprintf(config, "\t(joystick   %d)\n", joystick_num);

        fprintf(config, "\t(joystick-x   %d)\n", joystick_keymap.x_axis);
        fprintf(config, "\t(joystick-y   %d)\n", joystick_keymap.y_axis);
        fprintf(config, "\t(joystick-a   %d)\n", joystick_keymap.a_button);
        fprintf(config, "\t(joystick-b   %d)\n", joystick_keymap.b_button);
        fprintf(config, "\t(joystick-start  %d)\n", joystick_keymap.start_button);
        fprintf(config, "\t(joystick-deadzone  %d)\n", joystick_keymap.dead_zone);
        }
	
	customsave(config);

      fprintf(config, ")\n");
      fclose(config);
    }
}

