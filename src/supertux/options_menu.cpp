//  SuperTux
//  Copyright (C) 2004 Tobas Glaesser <tobi.web@gmx.de>
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "audio/sound_manager.hpp"
#include "control/joystickkeyboardcontroller.hpp"
#include "gui/menu.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/main.hpp"
#include "supertux/profile_menu.hpp"
#include "util/gettext.hpp"
#include "video/renderer.hpp"

Menu* options_menu   = 0;

enum OptionsMenuIDs {
  MNID_FULLSCREEN,
  MNID_FULLSCREEN_RESOLUTION,
  MNID_MAGNIFICATION,
  MNID_ASPECTRATIO,
  MNID_PROFILES,
  MNID_SOUND,
  MNID_MUSIC
};

class LanguageMenu : public Menu
{
public:
  LanguageMenu() {
    add_label(_("Language"));
    add_hl();
    add_entry(0, std::string("<")+_("auto-detect")+">");
    add_entry(1, "English");

    int mnid = 10;    
    std::set<std::string> languages = dictionary_manager.get_languages();
    for (std::set<std::string>::iterator i = languages.begin(); i != languages.end(); i++) {
      std::string locale_name = *i;
      TinyGetText::LanguageDef ldef = TinyGetText::get_language_def(locale_name);
      std::string locale_fullname = locale_name;
      if (std::string(ldef.code) == locale_name) {
        locale_fullname = ldef.name;
      }
      add_entry(mnid++, locale_fullname);
    } 

    add_hl();
    add_back(_("Back"));
  }

  virtual void menu_action(MenuItem* item) {
    if (item->id == 0) {
      config->locale = "";
      dictionary_manager.set_language(config->locale);
      config->save();
      Menu::pop_current();
    }
    else if (item->id == 1) {
      config->locale = "en";
      dictionary_manager.set_language(config->locale);
      config->save();
      Menu::pop_current();
    }
    int mnid = 10;    
    std::set<std::string> languages = dictionary_manager.get_languages();
    for (std::set<std::string>::iterator i = languages.begin(); i != languages.end(); i++) {
      std::string locale_name = *i;
      if (item->id == mnid++) {
        config->locale = locale_name;
        dictionary_manager.set_language(config->locale);
        config->save();
        Menu::pop_current();
      }
    }
  }
};

class OptionsMenu : public Menu
{
public:
  OptionsMenu();
  virtual ~OptionsMenu();

  virtual void menu_action(MenuItem* item);

protected:
  std::auto_ptr<LanguageMenu> language_menu;
  
};

OptionsMenu::OptionsMenu()
{
  language_menu.reset(new LanguageMenu());

  add_label(_("Options"));
  add_hl();

  // Language change should only be possible in the main menu, since elsewhere it might not always work fully
  // FIXME: Implement me: if (get_parent() == main_menu)
  add_submenu(_("Select Language"), language_menu.get())
    ->set_help(_("Select a different language to display text in"));

  add_submenu(_("Select Profile"), get_profile_menu())
    ->set_help(_("Select a profile to play with"));

  add_toggle(MNID_PROFILES, _("Profile on Startup"), config->sound_enabled)
    ->set_help(_("Select your profile immediately after start-up"));
  
  add_toggle(MNID_FULLSCREEN,_("Fullscreen"), config->use_fullscreen)
    ->set_help(_("Fill the entire screen"));

  MenuItem* fullscreen_res = add_string_select(MNID_FULLSCREEN_RESOLUTION, _("Resolution"));
  fullscreen_res->set_help(_("Determine the resolution used in fullscreen mode (you must toggle fullscreen to complete the change)"));

  MenuItem* magnification = add_string_select(MNID_MAGNIFICATION, _("Magnification"));
  magnification->set_help(_("Change the magnification of the game area"));

  // These values go from screen:640/projection:1600 to
  // screen:1600/projection:640 (i.e. 640, 800, 1024, 1280, 1600)
  magnification->list.push_back("auto");
  magnification->list.push_back("40%");
  magnification->list.push_back("50%");
  magnification->list.push_back("62.5%");
  magnification->list.push_back("80%");
  magnification->list.push_back("100%");
  magnification->list.push_back("125%");
  magnification->list.push_back("160%");
  magnification->list.push_back("200%");
  magnification->list.push_back("250%");

  SDL_Rect** modes = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_OPENGL);

  if (modes == (SDL_Rect **)0) 
    { // No resolutions at all available, bad

    }
  else if(modes == (SDL_Rect **)-1) 
    { // All resolutions should work, so we fall back to hardcoded defaults
      fullscreen_res->list.push_back("640x480");
      fullscreen_res->list.push_back("800x600");
      fullscreen_res->list.push_back("1024x768");
      fullscreen_res->list.push_back("1152x864");
      fullscreen_res->list.push_back("1280x960");
      fullscreen_res->list.push_back("1280x1024");
      fullscreen_res->list.push_back("1440x900");
      fullscreen_res->list.push_back("1680x1050");
      fullscreen_res->list.push_back("1600x1200");
      fullscreen_res->list.push_back("1920x1080");
      fullscreen_res->list.push_back("1920x1200");
    }
  else 
    {
      for(int i = 0; modes[i]; ++i)
        {
          std::ostringstream out;          
          out << modes[i]->w << "x" << modes[i]->h;
          fullscreen_res->list.push_back(out.str());
        }
    }

  MenuItem* aspect = add_string_select(MNID_ASPECTRATIO, _("Aspect Ratio"));
  aspect->set_help(_("Adjust the aspect ratio"));
  
  aspect->list.push_back("auto");
  aspect->list.push_back("5:4");
  aspect->list.push_back("4:3");
  aspect->list.push_back("16:10");
  aspect->list.push_back("16:9");
  aspect->list.push_back("1368:768");

  if (config->aspect_width != 0 && config->aspect_height != 0)
    {
      std::ostringstream out;
      out << config->aspect_width << ":" << config->aspect_height;
      std::string aspect_ratio = out.str();
      for(std::vector<std::string>::iterator i = aspect->list.begin(); i != aspect->list.end(); ++i)
        {
          if(*i == aspect_ratio)
            {
              aspect_ratio.clear();
              break;
            }
        }

      if (!aspect_ratio.empty())
        {
          aspect->selected = aspect->list.size();
          aspect->list.push_back(aspect_ratio);
        }
    }
  
  if (sound_manager->is_audio_enabled()) {
    add_toggle(MNID_SOUND, _("Sound"), config->sound_enabled)
      ->set_help(_("Disable all sound effects"));
    add_toggle(MNID_MUSIC, _("Music"), config->music_enabled)
      ->set_help(_("Disable all music"));
  } else {
    add_inactive(MNID_SOUND, _("Sound (disabled)"));
    add_inactive(MNID_MUSIC, _("Music (disabled)"));
  }
  
  add_submenu(_("Setup Keyboard"), main_controller->get_key_options_menu())
    ->set_help(_("Configure key-action mappings"));

  add_submenu(_("Setup Joystick"),main_controller->get_joystick_options_menu())
    ->set_help(_("Configure joystick control-action mappings"));
  add_hl();
  add_back(_("Back"));
}

OptionsMenu::~OptionsMenu()
{
}

void
OptionsMenu::menu_action(MenuItem* item)
{
  switch (item->id) {
    case MNID_ASPECTRATIO:
      { 
        if (item->list[item->selected] == "auto")
          {
            config->aspect_width  = 0; // Magic values
            config->aspect_height = 0;
            Renderer::instance()->apply_config();
            Menu::recalc_pos();
          }
        else if(sscanf(item->list[item->selected].c_str(), "%d:%d", &config->aspect_width, &config->aspect_height) == 2)
          {
            Renderer::instance()->apply_config();
            Menu::recalc_pos();
          }
        else
          {
            assert(!"This must not be reached");
          }
      }
      break;

    case MNID_MAGNIFICATION:
      if (item->list[item->selected] == "auto")
        {
          config->magnification = 0.0f; // Magic value 
        }
      else if(sscanf(item->list[item->selected].c_str(), "%f", &config->magnification) == 1)
        {
          config->magnification /= 100.0f;
        }
      Renderer::instance()->apply_config();
      Menu::recalc_pos();
      break;

    case MNID_FULLSCREEN_RESOLUTION:
      if(sscanf(item->list[item->selected].c_str(), "%dx%d", &config->fullscreen_width, &config->fullscreen_height) == 2)
        {
          // do nothing, changes are only applied when toggling fullscreen mode
        }      
      break;

    case MNID_FULLSCREEN:
      if(config->use_fullscreen != options_menu->is_toggled(MNID_FULLSCREEN)) {
        config->use_fullscreen = !config->use_fullscreen;
        init_video(); // FIXME: Should call apply_config instead
        Menu::recalc_pos();
        config->save();
      }
      break;

    case MNID_SOUND:
      if(config->sound_enabled != options_menu->is_toggled(MNID_SOUND)) {
        config->sound_enabled = !config->sound_enabled;
        sound_manager->enable_sound(config->sound_enabled);
        config->save();
      }
      break;

    case MNID_MUSIC:
      if(config->music_enabled != options_menu->is_toggled(MNID_MUSIC)) {
        config->music_enabled = !config->music_enabled;
        sound_manager->enable_music(config->music_enabled);
        config->save();
      }
      break;

    default:
      break;
  }
}

Menu* get_options_menu()
{
  //static OptionsMenu menu;
  options_menu = new OptionsMenu();
  return options_menu;
}

void free_options_menu()
{
  delete options_menu;
  options_menu = 0;
}

/* EOF */
