//  SuperTux -- LevelIntro screen
//  Copyright (C) 2008 Christoph Sommer <christoph.sommer@2008.expires.deltadevelopment.de>
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

#include "supertux/levelintro.hpp"

#include "control/input_manager.hpp"
#include "math/random_generator.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/fadeout.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "supertux/resources.hpp"
#include "supertux/player_status.hpp"
#include "util/gettext.hpp"

#include <sstream>
#include <boost/format.hpp>

LevelIntro::LevelIntro(const Level* level_, const Statistics* best_level_statistics_, const PlayerStatus* player_status_) :
  level(level_),
  best_level_statistics(best_level_statistics_),
  player_sprite(SpriteManager::current()->create("images/creatures/tux/tux.sprite")),
  power_sprite(SpriteManager::current()->create("images/creatures/tux/powerups.sprite")),
  player_sprite_py(0),
  player_sprite_vy(0),
  player_sprite_jump_timer(),
  player_status(player_status_)
{
  //Show appropriate tux animation for player status.
  player_sprite->set_action(player_status->get_bonus_prefix() + "-walk-right");
  player_sprite_jump_timer.start(graphicsRandom.randf(5,10));

  /* Set Tux powerup sprite action */
  if (player_status->bonus == EARTH_BONUS
      || player_status->bonus == AIR_BONUS
      || (player_status->bonus == FIRE_BONUS && g_config->christmas_mode))
  {
    power_sprite->set_action(player_sprite->get_action());
  }
}

LevelIntro::~LevelIntro()
{
}

void
LevelIntro::setup()
{
}

void
LevelIntro::update(float elapsed_time)
{
  Controller *controller = InputManager::current()->get_controller();

  // Check if it's time to exit the screen
  if(controller->pressed(Controller::JUMP)
     || controller->pressed(Controller::ACTION)
     || controller->pressed(Controller::MENU_SELECT)
     || controller->pressed(Controller::START)
     || controller->pressed(Controller::ESCAPE)) {
    if(g_config->transitions_enabled)
    {
      ScreenManager::current()->pop_screen(std::unique_ptr<ScreenFade>(new FadeOut(0.1)));
    }
    else
    {
      ScreenManager::current()->pop_screen();
    }
  }

  player_sprite_py += player_sprite_vy * elapsed_time;
  player_sprite_vy += 100 * elapsed_time * Sector::current()->get_gravity();
  if (player_sprite_py >= 0) {
    player_sprite_py = 0;
    player_sprite_vy = 0;
    player_sprite->set_action(player_status->get_bonus_prefix() + "-walk-right");
  } else {

    player_sprite->set_action(player_status->get_bonus_prefix() + "-jump-right");
  }
  if (player_sprite_jump_timer.check()) {
    player_sprite_vy = -300;
    player_sprite_jump_timer.start(graphicsRandom.randf(2,3));
  }

}

void
LevelIntro::draw(DrawingContext& context)
{
  const Statistics& stats = level->stats;
  int py = static_cast<int>(SCREEN_HEIGHT / 2 - Resources::normal_font->get_height() / 2);

  context.draw_filled_rect(Vector(0, 0), Vector(SCREEN_WIDTH, SCREEN_HEIGHT), Color(0.0f, 0.0f, 0.0f, 1.0f), 0);

  {
    context.draw_center_text(Resources::normal_font, level->get_name(), Vector(0, py), LAYER_FOREGROUND1, LevelIntro::header_color);
    py += static_cast<int>(Resources::normal_font->get_height());
  }

  std::string author = level->get_author();
  if ((!author.empty()) && (author != "SuperTux Team")) {
    std::string author_text = str(boost::format(_("contributed by %s")) % author);
    context.draw_center_text(Resources::small_font, author_text, Vector(0, py), LAYER_FOREGROUND1, LevelIntro::author_color);
    py += static_cast<int>(Resources::small_font->get_height());
  }

  py += 32;

  {
    player_sprite->draw(context, Vector((SCREEN_WIDTH - player_sprite->get_current_hitbox_width()) / 2, py + player_sprite_py), LAYER_FOREGROUND1);
    if (player_status->bonus == EARTH_BONUS
        || player_status->bonus == AIR_BONUS
        || (player_status->bonus == FIRE_BONUS && g_config->christmas_mode))
    {
      power_sprite->draw(context, Vector((SCREEN_WIDTH - player_sprite->get_current_hitbox_width()) / 2, py + player_sprite_py), LAYER_FOREGROUND1);
    }
    py += static_cast<int>(player_sprite->get_current_hitbox_height());
  }

  py += 32;

  {
    context.draw_center_text(Resources::normal_font, std::string("- ") + _("Best Level Statistics") + std::string(" -"), Vector(0, py), LAYER_FOREGROUND1, LevelIntro::stat_hdr_color);
    py += static_cast<int>(Resources::normal_font->get_height());
  }

  {
    std::stringstream ss;
    ss << _("Coins") << ": " << Statistics::coins_to_string((best_level_statistics && (best_level_statistics->coins >= 0)) ? best_level_statistics->coins : 0, stats.total_coins);
    context.draw_center_text(Resources::normal_font, ss.str(), Vector(0, py), LAYER_FOREGROUND1, LevelIntro::stat_color);
    py += static_cast<int>(Resources::normal_font->get_height());
  }

  {
    std::stringstream ss;
    ss << _("Badguys killed") << ": " << Statistics::frags_to_string((best_level_statistics && (best_level_statistics->coins >= 0)) ? best_level_statistics->badguys : 0, stats.total_badguys);
    context.draw_center_text(Resources::normal_font, ss.str(), Vector(0, py), LAYER_FOREGROUND1,LevelIntro::stat_color);
    py += static_cast<int>(Resources::normal_font->get_height());
  }

  {
    std::stringstream ss;
    ss << _("Secrets") << ": " << Statistics::secrets_to_string((best_level_statistics && (best_level_statistics->coins >= 0)) ? best_level_statistics->secrets : 0, stats.total_secrets);
    context.draw_center_text(Resources::normal_font, ss.str(), Vector(0, py), LAYER_FOREGROUND1,LevelIntro::stat_color);
    py += static_cast<int>(Resources::normal_font->get_height());
  }

  {
    std::stringstream ss;
    ss << _("Best time") << ": " << Statistics::time_to_string((best_level_statistics && (best_level_statistics->coins >= 0)) ? best_level_statistics->time : 0);
    context.draw_center_text(Resources::normal_font, ss.str(), Vector(0, py), LAYER_FOREGROUND1,LevelIntro::stat_color);
    py += static_cast<int>(Resources::normal_font->get_height());
  }

  if(level->target_time){
    std::stringstream ss;
    ss << _("Level target time") << ": " << Statistics::time_to_string(level->target_time);
    context.draw_center_text(Resources::normal_font, ss.str(), Vector(0, py), LAYER_FOREGROUND1,LevelIntro::stat_color);
  }

}

/* EOF */
