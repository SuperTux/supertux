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

#include "supertux/menu/options_menu.hpp"

#include "audio/sound_manager.hpp"
#include "gui/dialog.hpp"
#include "gui/item_goto.hpp"
#include "gui/item_stringselect.hpp"
#include "gui/item_toggle.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/game_session.hpp"
#include "supertux/globals.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"
#include "video/renderer.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

namespace {

bool less_than_volume(const std::string& lhs, const std::string& rhs) {
  int lhs_i, rhs_i;
  if (sscanf(lhs.c_str(), "%i", &lhs_i) == 1 &&
      sscanf(rhs.c_str(), "%i", &rhs_i) == 1)
  {
    return lhs_i < rhs_i;
  }

  return false;
}


} // namespace

enum OptionsMenuIDs {
  MNID_WINDOW_RESIZABLE,
  MNID_WINDOW_RESOLUTION,
  MNID_FULLSCREEN,
  MNID_FULLSCREEN_RESOLUTION,
#ifdef __EMSCRIPTEN__
  MNID_FIT_WINDOW,
#endif
  MNID_MAGNIFICATION,
  MNID_ASPECTRATIO,
  MNID_VSYNC,
  MNID_SOUND,
  MNID_MUSIC,
  MNID_SOUND_VOLUME,
  MNID_MUSIC_VOLUME,
  MNID_RUMBLING,
  MNID_DEVELOPER_MODE,
  MNID_CHRISTMAS_MODE,
  MNID_TRANSITIONS,
  MNID_CONFIRMATION_DIALOG,
  MNID_PAUSE_ON_FOCUSLOSS,
  MNID_CUSTOM_CURSOR,
  MNID_DISABLE_NETWORK,
#ifndef __EMSCRIPTEN__
  MNID_RELEASE_CHECK,
#endif
  MNID_MOBILE_CONTROLS,
  MNID_MOBILE_CONTROLS_SCALE
};

OptionsMenu::OptionsMenu(bool complete) :
  next_magnification(0),
  next_aspect_ratio(0),
  next_window_resolution(0),
  next_resolution(0),
  next_vsync(0),
  next_sound_volume(0),
  next_music_volume(0),
  m_next_mobile_controls_scale(0),
  magnifications(),
  aspect_ratios(),
  window_resolutions(),
  resolutions(),
  vsyncs(),
  sound_volumes(),
  music_volumes(),
  m_mobile_controls_scales()
{
  add_label(_("Options"));
  add_hl();

  magnifications.clear();
  // These values go from screen:640/projection:1600 to
  // screen:1600/projection:640 (i.e. 640, 800, 1024, 1280, 1600)
  magnifications.push_back(_("auto"));
#ifndef HIDE_NONMOBILE_OPTIONS
  magnifications.push_back("40%");
  magnifications.push_back("50%");
  magnifications.push_back("62.5%");
  magnifications.push_back("80%");
#endif
  magnifications.push_back("100%");
  magnifications.push_back("125%");
  magnifications.push_back("160%");
  magnifications.push_back("200%");
  magnifications.push_back("250%");
  // Gets the actual magnification:
  if (g_config->magnification != 0.0f) //auto
  {
    std::ostringstream out;
    out << (g_config->magnification*100) << "%";
    std::string magn = out.str();
    int count = 0;
    for (const auto& magnification : magnifications)
    {
      if (magnification == magn)
      {
        next_magnification = count;
        magn.clear();
        break;
      }

      ++count;
    }
    if (!magn.empty()) //magnification not in our list but accept anyway
    {
      next_magnification = static_cast<int>(magnifications.size());
      magnifications.push_back(magn);
    }
  }

  aspect_ratios.clear();
  aspect_ratios.push_back(_("auto"));
  aspect_ratios.push_back("5:4");
  aspect_ratios.push_back("4:3");
  aspect_ratios.push_back("16:10");
  aspect_ratios.push_back("16:9");
  aspect_ratios.push_back("1368:768");
  // Gets the actual aspect ratio:
  if (g_config->aspect_size != Size(0, 0)) //auto
  {
    std::ostringstream out;
    out << g_config->aspect_size.width << ":" << g_config->aspect_size.height;
    std::string aspect_ratio = out.str();
    int cnt_ = 0;
    for (const auto& ratio : aspect_ratios)
    {
      if (ratio == aspect_ratio)
      {
        aspect_ratio.clear();
        next_aspect_ratio = cnt_;
        break;
      }
      ++cnt_;
    }

    if (!aspect_ratio.empty())
    {
      next_aspect_ratio = static_cast<int>(aspect_ratios.size());
      aspect_ratios.push_back(aspect_ratio);
    }
  }

  {
    window_resolutions = { "640x480", "854x480", "800x600", "1280x720", "1280x800",
                           "1440x900", "1920x1080", "1920x1200", "2560x1440" };
    next_window_resolution = -1;
    Size window_size = VideoSystem::current()->get_window_size();
    std::ostringstream out;
    out << window_size.width << "x" << window_size.height;
    std::string window_size_text = out.str();
    for (size_t i = 0; i < window_resolutions.size(); ++i)
    {
      if (window_resolutions[i] == window_size_text)
      {
        next_window_resolution = static_cast<int>(i);
        break;
      }
    }
    if (next_window_resolution == -1)
    {
      window_resolutions.insert(window_resolutions.begin(), window_size_text);
      next_window_resolution = 0;
    }
  }

  resolutions.clear();
  int display_mode_count = SDL_GetNumDisplayModes(0);
  std::string last_display_mode;
  for (int i = 0; i < display_mode_count; ++i)
  {
    SDL_DisplayMode mode;
    int ret = SDL_GetDisplayMode(0, i, &mode);
    if (ret != 0)
    {
      log_warning << "failed to get display mode: " << SDL_GetError() << std::endl;
    }
    else
    {
      std::ostringstream out;
      out << mode.w << "x" << mode.h;
      if (mode.refresh_rate)
        out << "@" << mode.refresh_rate;
      if (last_display_mode == out.str())
        continue;
      last_display_mode = out.str();
      resolutions.insert(resolutions.begin(), out.str());
    }
  }
  resolutions.push_back("Desktop");

  std::string fullscreen_size_str = _("Desktop");
  {
    std::ostringstream out;
    if (g_config->fullscreen_size != Size(0, 0))
    {
      out << g_config->fullscreen_size.width << "x" << g_config->fullscreen_size.height;
      if (g_config->fullscreen_refresh_rate)
         out << "@" << g_config->fullscreen_refresh_rate;
      fullscreen_size_str = out.str();
    }
  }

  int cnt = 0;
  for (const auto& res : resolutions)
  {
    if (res == fullscreen_size_str)
    {
      fullscreen_size_str.clear();
      next_resolution = cnt;
      break;
    }
    ++cnt;
  }
  if (!fullscreen_size_str.empty())
  {
    next_resolution = static_cast<int>(resolutions.size());
    resolutions.push_back(fullscreen_size_str);
  }

  { // vsync
    vsyncs.push_back(_("on"));
    vsyncs.push_back(_("off"));
    vsyncs.push_back(_("adaptive"));
    int mode = VideoSystem::current()->get_vsync();

    switch (mode)
    {
      case -1:
        next_vsync = 2;
        break;

      case 0:
        next_vsync = 1;
        break;

      case 1:
        next_vsync = 0;
        break;

      default:
        log_warning << "Unknown swap mode: " << mode << std::endl;
        next_vsync = 0;
    }
  }

  // Sound Volume
  sound_volumes.clear();
  for (const char* percent : {"0%", "10%", "20%", "30%", "40%", "50%", "60%", "70%", "80%", "90%", "100%"}) {
    sound_volumes.push_back(percent);
  }

  std::ostringstream sound_vol_stream;
  sound_vol_stream << g_config->sound_volume << "%";
  std::string sound_vol_string = sound_vol_stream.str();

  if (std::find(sound_volumes.begin(),
               sound_volumes.end(), sound_vol_string) == sound_volumes.end())
  {
    sound_volumes.push_back(sound_vol_string);
  }

  std::sort(sound_volumes.begin(), sound_volumes.end(), less_than_volume);

  std::ostringstream out;
  out << g_config->sound_volume << "%";
  std::string sound_volume = out.str();
  int cnt_ = 0;
  for (const auto& volume : sound_volumes)
  {
    if (volume == sound_volume)
    {
      sound_volume.clear();
      next_sound_volume = cnt_;
      break;
    }
    ++cnt_;
  }

  // Music Volume
  music_volumes.clear();
  for (const char* percent : {"0%", "10%", "20%", "30%", "40%", "50%", "60%", "70%", "80%", "90%", "100%"}) {
    music_volumes.push_back(percent);
  }

  std::ostringstream music_vol_stream;
  music_vol_stream << g_config->music_volume << "%";
  std::string music_vol_string = music_vol_stream.str();

  if (std::find(music_volumes.begin(),
               music_volumes.end(), music_vol_string) == music_volumes.end())
  {
    music_volumes.push_back(music_vol_string);
  }

  std::sort(music_volumes.begin(), music_volumes.end(), less_than_volume);

  out.str("");
  out.clear();
  out << g_config->music_volume << "%";
  std::string music_volume = out.str();
  cnt_ = 0;
  for (const auto& volume : music_volumes)
  {
    if (volume == music_volume)
    {
      music_volume.clear();
      next_music_volume = cnt_;
      break;
    }
    ++cnt_;
  }

  if (complete)
  {
    // Language and profile changes are only be possible in the
    // main menu, since elsewhere it might not always work fully
    add_submenu(_("Select Language"), MenuStorage::LANGUAGE_MENU)
      .set_help(_("Select a different language to display text in"));

    add_submenu(_("Language Packs"), MenuStorage::LANGPACK_MENU)
      .set_help(_("Language packs contain up-to-date translations"));

    add_submenu(_("Select Profile"), MenuStorage::PROFILE_MENU)
      .set_help(_("Select a profile to play with"));
  }

#if !defined(HIDE_NONMOBILE_OPTIONS) && !defined(__EMSCRIPTEN__)
  add_toggle(MNID_FULLSCREEN,_("Window Resizable"), &g_config->window_resizable)
    .set_help(_("Allow window resizing, might require a restart to take effect"));

  MenuItem& window_res = add_string_select(MNID_WINDOW_RESOLUTION, _("Window Resolution"), &next_window_resolution, window_resolutions);
  window_res.set_help(_("Resize the window to the given size"));

  add_toggle(MNID_FULLSCREEN,_("Fullscreen"), &g_config->use_fullscreen)
    .set_help(_("Fill the entire screen"));

  MenuItem& fullscreen_res = add_string_select(MNID_FULLSCREEN_RESOLUTION, _("Fullscreen Resolution"), &next_resolution, resolutions);
  fullscreen_res.set_help(_("Determine the resolution used in fullscreen mode (you must toggle fullscreen to complete the change)"));
#endif

#if 0
#ifdef __EMSCRIPTEN__
  MenuItem& fit_window = add_toggle(MNID_FIT_WINDOW, _("Fit to browser"), &g_config->fit_window);
  fit_window.set_help(_("Fit the resolution to the size of your browser"));
#endif
#endif

  MenuItem& magnification = add_string_select(MNID_MAGNIFICATION, _("Magnification"), &next_magnification, magnifications);
  magnification.set_help(_("Change the magnification of the game area"));

  MenuItem& vsync = add_string_select(MNID_VSYNC, _("VSync"), &next_vsync, vsyncs);
  vsync.set_help(_("Set the VSync mode"));

#if !defined(HIDE_NONMOBILE_OPTIONS) && !defined(__EMSCRIPTEN__)
  MenuItem& aspect = add_string_select(MNID_ASPECTRATIO, _("Aspect Ratio"), &next_aspect_ratio, aspect_ratios);
  aspect.set_help(_("Adjust the aspect ratio"));
#endif

  MenuItem& video_system_menu = add_submenu(_("Change Video System"), MenuStorage::MenuId::VIDEO_SYSTEM_MENU);
  video_system_menu.set_help(_("Change video system used to render graphics"));

  if (SoundManager::current()->is_audio_enabled())
  {
    add_toggle(MNID_SOUND, _("Sound"), &g_config->sound_enabled)
      .set_help(_("Disable all sound effects"));
    add_toggle(MNID_MUSIC, _("Music"), &g_config->music_enabled)
      .set_help(_("Disable all music"));

    MenuItem& sound_volume_select = add_string_select(MNID_SOUND_VOLUME, _("Sound Volume"), &next_sound_volume, sound_volumes);
    sound_volume_select.set_help(_("Adjust sound volume"));

    MenuItem& music_volume_select = add_string_select(MNID_MUSIC_VOLUME, _("Music Volume"), &next_music_volume, music_volumes);
    music_volume_select.set_help(_("Adjust music volume"));
  }
  else
  {
    add_inactive( _("Sound (disabled)"));
    add_inactive( _("Music (disabled)"));
  }

  // Separated both translation strings so the latter can be removed if it is
  // no longer true, without requiring a new round of translating
  add_toggle(MNID_RUMBLING, _("Enable Rumbling Controllers"), &g_config->multiplayer_buzz_controllers)
    .set_help(_("Enable vibrating the game controllers.") + " " + _("This feature is currently only used in the multiplayer options menu."));

  add_submenu(_("Setup Keyboard"), MenuStorage::KEYBOARD_MENU)
    .set_help(_("Configure key-action mappings"));

#ifndef UBUNTU_TOUCH
  add_submenu(_("Setup Joystick"), MenuStorage::JOYSTICK_MENU)
    .set_help(_("Configure joystick control-action mappings"));

  add_submenu(_("Multiplayer settings"), MenuStorage::MULTIPLAYER_MENU)
    .set_help(_("Configure settings specific to multiplayer"));
#endif

  add_toggle(MNID_MOBILE_CONTROLS, _("On-screen controls"), &g_config->mobile_controls)
      .set_help(_("Toggle on-screen controls for mobile devices"));
  m_mobile_controls_scales.clear();
  for (unsigned i = 50; i <= 300; i+=25)
  {
    m_mobile_controls_scales.push_back(std::to_string(i) + "%");
    if (i == static_cast<unsigned>(g_config->m_mobile_controls_scale * 100))
      m_next_mobile_controls_scale = (i - 50) / 25;
  }
  add_string_select(MNID_MOBILE_CONTROLS_SCALE, _("On-screen controls scale"), &m_next_mobile_controls_scale, m_mobile_controls_scales);

  MenuItem& enable_transitions = add_toggle(MNID_TRANSITIONS, _("Enable transitions"), &g_config->transitions_enabled);
  enable_transitions.set_help(_("Enable screen transitions and smooth menu animation"));

#ifndef HIDE_NONMOBILE_OPTIONS
  if (g_config->developer_mode)
  {
    add_toggle(MNID_DEVELOPER_MODE, _("Developer Mode"), &g_config->developer_mode);
  }
#else
  add_toggle(MNID_DEVELOPER_MODE, _("Developer Mode"), &g_config->developer_mode);
#endif

  if (g_config->is_christmas() || g_config->christmas_mode)
  {
    add_toggle(MNID_CHRISTMAS_MODE, _("Christmas Mode"), &g_config->christmas_mode);
  }

  add_toggle(MNID_CONFIRMATION_DIALOG, _("Confirmation Dialog"), &g_config->confirmation_dialog).set_help(_("Confirm aborting level"));
  add_toggle(MNID_PAUSE_ON_FOCUSLOSS, _("Pause on focus loss"), &g_config->pause_on_focusloss)
    .set_help(_("Automatically pause the game when the window loses focus"));
  add_toggle(MNID_CUSTOM_CURSOR, _("Use custom mouse cursor"), &g_config->custom_mouse_cursor).set_help(_("Whether the game renders its own cursor or uses the system's cursor"));
#ifndef __EMSCRIPTEN__
  add_toggle(MNID_RELEASE_CHECK, _("Check for new releases"), &g_config->do_release_check)
    .set_help(_("Allows the game to perform checks for new SuperTux releases on startup and notify if any found."));
#endif

  add_toggle(MNID_DISABLE_NETWORK, _("Disable network"), &g_config->disable_network)
    .set_help(_("Prevents the game from connecting online"));

  add_submenu(_("Integrations and presence"), MenuStorage::INTEGRATIONS_MENU)
      .set_help(_("Manage whether SuperTux should display the levels you play on your social media profiles (Discord)"));

  if (g_config->developer_mode)
  {
    add_submenu(_("Menu Customization"), MenuStorage::CUSTOM_MENU_MENU)
      .set_help(_("Customize the appearance of the menus"));
  }

  add_hl();
  add_back(_("Back"));
}

OptionsMenu::~OptionsMenu()
{
}

void
OptionsMenu::menu_action(MenuItem& item)
{
  switch (item.get_id()) {
    case MNID_ASPECTRATIO:
      {
        if (aspect_ratios[next_aspect_ratio] == _("auto"))
        {
          g_config->aspect_size = Size(0, 0); // Magic values
          VideoSystem::current()->apply_config();
          MenuManager::instance().on_window_resize();
        }
        else if (sscanf(aspect_ratios[next_aspect_ratio].c_str(), "%d:%d",
                        &g_config->aspect_size.width, &g_config->aspect_size.height) == 2)
        {
          VideoSystem::current()->apply_config();
          MenuManager::instance().on_window_resize();
        }
        else
        {
          log_fatal << "Invalid aspect ratio " << aspect_ratios[next_aspect_ratio] << " specified" << std::endl;
          assert(false);
        }
      }
      break;

    case MNID_MAGNIFICATION:
      if (magnifications[next_magnification] == _("auto"))
      {
        g_config->magnification = 0.0f; // Magic value
      }
      else if (sscanf(magnifications[next_magnification].c_str(), "%f", &g_config->magnification) == 1)
      {
        g_config->magnification /= 100.0f;
      }
      VideoSystem::current()->apply_config();
      MenuManager::instance().on_window_resize();
      break;

    case MNID_WINDOW_RESIZABLE:
      if (!g_config->window_resizable) {
        next_window_resolution = 0;
      }
      break;

    case MNID_WINDOW_RESOLUTION:
      {
        int width;
        int height;
        if (sscanf(window_resolutions[next_window_resolution].c_str(), "%dx%d",
                   &width, &height) != 2)
        {
          log_fatal << "can't parse " << window_resolutions[next_window_resolution] << std::endl;
        }
        else
        {
          g_config->window_size = Size(width, height);
          VideoSystem::current()->apply_config();
          MenuManager::instance().on_window_resize();
        }
      }
      break;

    case MNID_FULLSCREEN_RESOLUTION:
      {
        int width;
        int height;
        int refresh_rate;
        if (resolutions[next_resolution] == "Desktop")
        {
          g_config->fullscreen_size.width = 0;
          g_config->fullscreen_size.height = 0;
          g_config->fullscreen_refresh_rate = 0;
        }
        else if (sscanf(resolutions[next_resolution].c_str(), "%dx%d@%d",
                  &width, &height, &refresh_rate) == 3)
        {
          // do nothing, changes are only applied when toggling fullscreen mode
          g_config->fullscreen_size.width = width;
          g_config->fullscreen_size.height = height;
          g_config->fullscreen_refresh_rate = refresh_rate;
        }
        else if (sscanf(resolutions[next_resolution].c_str(), "%dx%d",
                       &width, &height) == 2)
        {
            g_config->fullscreen_size.width = width;
            g_config->fullscreen_size.height = height;
            g_config->fullscreen_refresh_rate = 0;
        }
      }
      break;

#ifdef __EMSCRIPTEN__
    case MNID_FIT_WINDOW:
      {
        // Emscripten's Clang detects the "$" in the macro as part of C++ code
        // although it isn't even Javascript, it's Emscripten's way to pass
        // arguments from C++ to Javascript
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdollar-in-identifier-extension"
        int resultds = EM_ASM_INT({
          if (window.supertux_setAutofit)
            window.supertux_setAutofit($0);

          return !!window.supertux_setAutofit;
        }, g_config->fit_window);
#pragma GCC diagnostic pop

        if (!resultds)
        {
          Dialog::show_message(_("The game couldn't detect your browser resolution.\n"
                                 "This most likely happens because it is not embedded\n"
                                 "in the SuperTux custom HTML template.\n"));
        }
      }
      break;
#endif

    case MNID_VSYNC:
      switch (next_vsync)
      {
        case 2:
          VideoSystem::current()->set_vsync(-1);
          break;

        case 1:
          VideoSystem::current()->set_vsync(0);
          break;

        case 0:
          VideoSystem::current()->set_vsync(1);
          break;

        default:
          assert(false);
          break;
      }
      break;

    case MNID_FULLSCREEN:
      VideoSystem::current()->apply_config();
      MenuManager::instance().on_window_resize();
      g_config->save();
      break;

    case MNID_SOUND:
      SoundManager::current()->enable_sound(g_config->sound_enabled);
      g_config->save();
      break;

    case MNID_SOUND_VOLUME:
      if (sscanf(sound_volumes[next_sound_volume].c_str(), "%i", &g_config->sound_volume) == 1)
      {
        bool sound_enabled = g_config->sound_volume > 0 ? true : false;
        SoundManager::current()->enable_sound(sound_enabled);
        SoundManager::current()->set_sound_volume(g_config->sound_volume);
        g_config->save();
      }
      break;

    case MNID_MUSIC:
      SoundManager::current()->enable_music(g_config->music_enabled);
      g_config->save();
      break;

    case MNID_MUSIC_VOLUME:
      if (sscanf(music_volumes[next_music_volume].c_str(), "%i", &g_config->music_volume) == 1)
      {
        bool music_enabled = g_config->music_volume > 0 ? true : false;
        SoundManager::current()->enable_music(music_enabled);
        SoundManager::current()->set_music_volume(g_config->music_volume);
        g_config->save();
      }
      break;

    case MNID_CUSTOM_CURSOR:
      SDL_ShowCursor(g_config->custom_mouse_cursor ? 0 : 1);
      break;

    case MNID_MOBILE_CONTROLS_SCALE:
      if (sscanf(m_mobile_controls_scales[m_next_mobile_controls_scale].c_str(), "%f", &g_config->m_mobile_controls_scale) == EOF)
        g_config->m_mobile_controls_scale = 1; // if sscanf fails revert to default scale
      else
        g_config->m_mobile_controls_scale /= 100.0f;
      break;

    default:
      break;
  }
}

/* EOF */
