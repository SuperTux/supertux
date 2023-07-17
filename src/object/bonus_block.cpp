//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#include "object/bonus_block.hpp"

#include <memory>

#include "audio/sound_manager.hpp"
#include "badguy/badguy.hpp"
#include "badguy/crusher.hpp"
#include "editor/editor.hpp"
#include "math/vector.hpp"
#include "object/bouncy_coin.hpp"
#include "object/coin_explode.hpp"
#include "object/coin_rain.hpp"
#include "object/flower.hpp"
#include "object/growup.hpp"
#include "object/oneup.hpp"
#include "object/player.hpp"
#include "object/portable.hpp"
#include "object/powerup.hpp"
#include "object/rock.hpp"
#include "object/specialriser.hpp"
#include "object/star.hpp"
#include "object/trampoline.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/constants.hpp"
#include "supertux/game_object_factory.hpp"
#include "supertux/level.hpp"
#include "supertux/sector.hpp"
#include "util/reader_collection.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"

namespace {

std::unique_ptr<MovingObject> to_moving_object(std::unique_ptr<GameObject> object)
{
  if (dynamic_cast<MovingObject*>(object.get()) != nullptr) {
    return std::unique_ptr<MovingObject>(static_cast<MovingObject*>(object.release()));
  } else {
    return std::unique_ptr<MovingObject>();
  }
}

const float upgrade_sound_gain = 0.3f;

} // namespace

BonusBlock::BonusBlock(const Vector& pos, int tile_data) :
  Block(SpriteManager::current()->create("images/objects/bonus_block/bonusblock.sprite")),
  m_contents(),
  m_object(),
  m_hit_counter(1),
  m_script(),
  m_lightsprite(),
  m_custom_sx()
{
  m_default_sprite_name = "images/objects/bonus_block/bonusblock.sprite";

  m_col.m_bbox.set_pos(pos);
  m_sprite->set_action("normal");
  m_contents = get_content_by_data(tile_data);
  preload_contents(tile_data);
}

BonusBlock::BonusBlock(const ReaderMapping& mapping) :
  Block(mapping, "images/objects/bonus_block/bonusblock.sprite"),
  m_contents(Content::COIN),
  m_object(),
  m_hit_counter(1),
  m_script(),
  m_lightsprite(),
  m_custom_sx()
{
  m_default_sprite_name = "images/objects/bonus_block/bonusblock.sprite";

  auto iter = mapping.get_iter();
  while (iter.next()) {
    const std::string& token = iter.get_key();
    if (token == "x" || token == "y" || token == "sprite") {
      // already initialized in Block::Block
    } else if (token == "count") {
      iter.get(m_hit_counter);
    } else if (token == "script") {
      iter.get(m_script);
    } else if (token == "data") {
      int d = 0;
      iter.get(d);
      m_contents = get_content_by_data(d);
      preload_contents(d);
    } else if (token == "contents") {
      std::string contentstring;
      iter.get(contentstring);

      m_contents = get_content_from_string(contentstring);

      if (m_contents == Content::CUSTOM)
      {
        if (Editor::is_active()) {
          mapping.get("custom-contents", m_custom_sx);
        } else {
          std::optional<ReaderCollection> content_collection;
          if (!mapping.get("custom-contents", content_collection))
          {
            log_warning << "bonusblock is missing 'custom-contents' tag" << std::endl;
          }
          else
          {
            const auto& object_specs = content_collection->get_objects();
            if (!object_specs.empty()) {
              if (object_specs.size() > 1) {
                log_warning << "only one object allowed in bonusblock 'custom-contents', ignoring the rest" << std::endl;
              }

              const ReaderObject& spec = object_specs[0];
              auto game_object = GameObjectFactory::instance().create(spec.get_name(), spec.get_mapping());
              m_object = to_moving_object(std::move(game_object));
              if (!m_object) {
                log_warning << "Only MovingObjects are allowed inside BonusBlocks" << std::endl;
              }
            }
          }
        }
      }
    } else if (token == "custom-contents") {
      // handled elsewhere
    } else {
      if (m_contents == Content::CUSTOM && !m_object) {
        // FIXME: This an ugly mess, could probably be removed as of
        // 16. Aug 2018 no level in either the supertux or the
        // addon-src repository is using this anymore
        ReaderMapping object_mapping = iter.as_mapping();
        auto game_object = GameObjectFactory::instance().create(token, object_mapping);

        m_object = to_moving_object(std::move(game_object));
        if (!m_object) {
          throw std::runtime_error("Only MovingObjects are allowed inside BonusBlocks");
        }
      } else {
        log_warning << "Invalid element '" << token << "' in bonusblock" << std::endl;
      }
    }
  }

  if (!Editor::is_active()) {
    if (m_contents == Content::CUSTOM && !m_object) {
      throw std::runtime_error("Need to specify content object for custom block");
    }
  }

  if (m_contents == Content::LIGHT || m_contents == Content::LIGHT_ON) {
    SoundManager::current()->preload("sounds/switch.ogg");
    m_lightsprite = Surface::from_file("/images/objects/lightmap_light/bonusblock_light.png");
    if (m_contents == Content::LIGHT_ON) {
      m_sprite->set_action("on");
    }
  }
}

BonusBlock::Content
BonusBlock::get_content_by_data(int tile_data) const
{
  // Warning: 'tile_data' can't be cast to 'Content', this manual
  // conversion is necessary
  switch (tile_data) {
    case 1: return Content::COIN;
    case 2: return Content::FIREGROW;
    case 3: return Content::STAR;
    case 4: return Content::ONEUP;
    case 5: return Content::ICEGROW;
    case 6: return Content::LIGHT;
    case 7: return Content::TRAMPOLINE;
    case 8: return Content::PORTABLE_TRAMPOLINE; // Trampoline
    case 9: return Content::ROCK; // Rock
    case 10: return Content::RAIN;
    case 11: return Content::EXPLODE;
    case 12: return Content::POTION; // Red potion
    case 13: return Content::AIRGROW;
    case 14: return Content::EARTHGROW;
    case 15: return Content::LIGHT_ON;
    default:
      log_warning << "Invalid box contents" << std::endl;
      return Content::COIN;
  }
}

BonusBlock::~BonusBlock()
{
}

ObjectSettings
BonusBlock::get_settings()
{
  ObjectSettings result = Block::get_settings();

  result.add_script(_("Script"), &m_script, "script");
  result.add_int(_("Count"), &m_hit_counter, "count", 1);
  result.add_enum(_("Content"), reinterpret_cast<int*>(&m_contents),
                  {_("Coin"), _("Growth (fire flower)"), _("Growth (ice flower)"), _("Growth (air flower)"),
                   _("Growth (earth flower)"), _("Star"), _("Tux doll"), _("Custom"), _("Script"), _("Light"), _("Light (On)"),
                   _("Trampoline"), _("Portable trampoline"), _("Coin rain"), _("Coin explosion"), _("Rock"), _("Potion")},
                  {"coin", "firegrow", "icegrow", "airgrow", "earthgrow", "star",
                   "1up", "custom", "script", "light", "light-on", "trampoline", "portabletrampoline", "rain", "explode", "rock", "potion"},
                  static_cast<int>(Content::COIN), "contents");
  result.add_sexp(_("Custom Content"), "custom-contents", m_custom_sx);

  result.reorder({"script", "count", "contents", "sprite", "x", "y"});

  return result;
}


void
BonusBlock::hit(Player& player)
{
  try_open(&player);
}

HitResponse
BonusBlock::collision(GameObject& other, const CollisionHit& hit_)
{
  auto player = dynamic_cast<Player*> (&other);
  if (player) {
    if (player->m_does_buttjump ||
      (player->is_swimboosting() && player->get_bbox().get_bottom() < m_col.m_bbox.get_top() + SHIFT_DELTA))
    {
      try_drop(player);
    }
  }

  auto badguy = dynamic_cast<BadGuy*> (&other);
  if (badguy) {
    // hit contains no information for collisions with blocks.
    // Badguy's bottom has to be below the top of the block
    // SHIFT_DELTA is required to slide over one tile gaps.
    if ( badguy->can_break() && ( badguy->get_bbox().get_bottom() > m_col.m_bbox.get_top() + SHIFT_DELTA ) ) {
      try_open(player);
    }
  }

  auto crusher = dynamic_cast<Crusher*> (&other);
  if (crusher)
  {
    try_open(player);
  }

  auto portable = dynamic_cast<Portable*> (&other);
  if (portable && !badguy) {
    auto moving = dynamic_cast<MovingObject*> (&other);
    if (moving->get_bbox().get_top() > m_col.m_bbox.get_bottom() - SHIFT_DELTA) {
      try_open(player);
    }
  }
  return Block::collision(other, hit_);
}

void
BonusBlock::try_open(Player* player)
{
  SoundManager::current()->play("sounds/brick.wav", get_pos());
  if (m_sprite->get_action() == "empty")
    return;

  if (player == nullptr)
    player = Sector::get().get_nearest_player(m_col.m_bbox);

  if (player == nullptr)
    return;

  Direction direction = (player->get_bbox().get_middle().x > m_col.m_bbox.get_middle().x) ? Direction::LEFT : Direction::RIGHT;

  bool play_upgrade_sound = false;
  switch (m_contents) {
    case Content::COIN:
    {
      Sector::get().add<BouncyCoin>(get_pos(), true);
      SoundManager::current()->play("sounds/coin.wav", get_pos());
      player->get_status().add_coins(1, false);
      if (m_hit_counter != 0 && !m_parent_dispenser)
        Sector::get().get_level().m_stats.increment_coins();
      break;
    }

    case Content::FIREGROW:
    {
      raise_growup_bonus(player, FIRE_BONUS, direction);
      break;
    }

    case Content::ICEGROW:
    {
      raise_growup_bonus(player, ICE_BONUS, direction);
      break;
    }

    case Content::AIRGROW:
    {
      raise_growup_bonus(player, AIR_BONUS, direction);
      break;
    }

    case Content::EARTHGROW:
    {
      raise_growup_bonus(player, EARTH_BONUS, direction);
      break;
    }

    case Content::STAR:
    {
      Sector::get().add<Star>(get_pos() + Vector(0, -32), direction);
      play_upgrade_sound = true;
      break;
    }

    case Content::ONEUP:
    {
      Sector::get().add<OneUp>(get_pos(), direction);
      play_upgrade_sound = true;
      break;
    }

    case Content::CUSTOM:
    {
      Sector::get().add<SpecialRiser>(get_pos(), std::move(m_object), true);
      play_upgrade_sound = true;
      break;
    }

    case Content::SCRIPT:
    { break; } // because scripts always run, this prevents default contents from being assumed

    case Content::LIGHT:
    case Content::LIGHT_ON:
    {
      if (m_sprite->get_action() == "on")
        m_sprite->set_action("off");
      else
        m_sprite->set_action("on");
      SoundManager::current()->play("sounds/switch.ogg", get_pos());
      break;
    }
    case Content::TRAMPOLINE:
    {
      Sector::get().add<SpecialRiser>(get_pos(), std::make_unique<Trampoline>(get_pos(), false), true);
      play_upgrade_sound = true;
      break;
    }
    case Content::PORTABLE_TRAMPOLINE:
    {
      Sector::get().add<SpecialRiser>(get_pos(), std::make_unique<Trampoline>(get_pos(), true), true);
      play_upgrade_sound = true;
      break;
    }
    case Content::ROCK:
    {
      Sector::get().add<SpecialRiser>(get_pos(), std::make_unique<Rock>(get_pos(), "images/objects/rock/rock.sprite"));
      break;
    }
    case Content::POTION:
    {
      Sector::get().add<SpecialRiser>(get_pos(), std::make_unique<PowerUp>(get_pos(), PowerUp::FLIP));
      break;
    }
    case Content::RAIN:
    {
      Sector::get().add<CoinRain>(get_pos(), true, !m_parent_dispenser);
      play_upgrade_sound = true;
      break;
    }
    case Content::EXPLODE:
    {
      Sector::get().add<CoinExplode>(get_pos() + Vector (0, -40), !m_parent_dispenser);
      play_upgrade_sound = true;
      break;
    }
  }

  if (play_upgrade_sound)
    SoundManager::current()->play("sounds/upgrade.wav", get_pos(), upgrade_sound_gain);

  if (!m_script.empty()) { // scripts always run if defined
    Sector::get().run_script(m_script, "BonusBlockScript");
  }

  start_bounce(player);
  if (m_hit_counter <= 0 || m_contents == Content::LIGHT || m_contents == Content::LIGHT_ON) { //use 0 to allow infinite hits
  } else if (m_hit_counter == 1) {
    m_sprite->set_action("empty");
  } else {
    m_hit_counter--;
  }
}

void
BonusBlock::try_drop(Player *player)
{
  SoundManager::current()->play("sounds/brick.wav", get_pos());
  if (m_sprite->get_action() == "empty")
    return;

  // First what's below the bonus block, if solid send it up anyway (excepting doll)
  Rectf dest_;
  dest_.set_left(m_col.m_bbox.get_left() + 1);
  dest_.set_top(m_col.m_bbox.get_bottom() + 1);
  dest_.set_right(m_col.m_bbox.get_right() - 1);
  dest_.set_bottom(dest_.get_top() + 30);

  if (!Sector::get().is_free_of_statics(dest_, this, true) && !(m_contents == Content::ONEUP))
  {
    try_open(player);
    return;
  }

  if (player == nullptr)
    player = Sector::get().get_nearest_player(m_col.m_bbox);

  if (player == nullptr)
    return;

  Direction direction = (player->get_bbox().get_middle().x > m_col.m_bbox.get_middle().x) ? Direction::LEFT : Direction::RIGHT;

  bool countdown = false;
  bool play_upgrade_sound = false;

  switch (m_contents) {
    case Content::COIN:
    {
      try_open(player);
      break;
    }

    case Content::FIREGROW:
    {
      drop_growup_bonus(player, PowerUp::FIRE, direction, countdown);
      break;
    }

    case Content::ICEGROW:
    {
      drop_growup_bonus(player, PowerUp::ICE, direction, countdown);
      break;
    }

    case Content::AIRGROW:
    {
      drop_growup_bonus(player, PowerUp::AIR, direction, countdown);
      break;
    }

    case Content::EARTHGROW:
    {
      drop_growup_bonus(player, PowerUp::EARTH, direction, countdown);
      break;
    }

    case Content::STAR:
    {
      Sector::get().add<Star>(get_pos() + Vector(0, 32), direction);
      play_upgrade_sound = true;
      countdown = true;
      break;
    }

    case Content::ONEUP:
    {
      Sector::get().add<OneUp>(get_pos(), Direction::DOWN);
      play_upgrade_sound = true;
      countdown = true;
      break;
    }

    case Content::CUSTOM:
    {
      //NOTE: non-portable trampolines could be moved to Content::CUSTOM, but they should not drop
      m_object->set_pos(get_pos() +  Vector(0, 32));
      Sector::get().add_object(std::move(m_object));
      play_upgrade_sound = true;
      countdown = true;
      break;
    }

    case Content::SCRIPT:
    {
      countdown = true;
      break;
    } // because scripts always run, this prevents default contents from being assumed

    case Content::LIGHT:
    case Content::LIGHT_ON:
    case Content::TRAMPOLINE:
    case Content::RAIN:
    {
      try_open(player);
      break;
    }
    case Content::ROCK:
    {
      Sector::get().add<Rock>(get_pos() + Vector(0, 32),  "images/objects/rock/rock.sprite");
      countdown = true;
      break;
    }
    case Content::PORTABLE_TRAMPOLINE:
    {
      Sector::get().add<Trampoline>(get_pos() + Vector(0, 32), true);
      countdown = true;
      break;
    }
    case Content::POTION:
    {
      Sector::get().add<PowerUp>(get_pos() + Vector(0, 32), PowerUp::FLIP);
      countdown = true;
      break;
    }
    case Content::EXPLODE:
    {
      Sector::get().add<CoinExplode>(get_pos() + Vector (0, 40), !m_parent_dispenser);
      play_upgrade_sound = true;
      countdown = true;
      break;
    }
  }

  if (play_upgrade_sound)
    SoundManager::current()->play("sounds/upgrade.wav", get_pos(), upgrade_sound_gain);

  if (!m_script.empty()) { // scripts always run if defined
    Sector::get().run_script(m_script, "powerup-script");
  }

  if (countdown) { // only decrease hit counter if try_open was not called
    if (m_hit_counter == 1) {
      m_sprite->set_action("empty");
    } else {
      m_hit_counter--;
    }
  }
}

void
BonusBlock::raise_growup_bonus(Player* player, const BonusType& bonus, const Direction& dir)
{
  std::unique_ptr<MovingObject> obj;
  if (player->get_status().bonus[player->get_id()] == NO_BONUS)
  {
    obj = std::make_unique<GrowUp>(get_pos(), dir);
  }
  else
  {
    obj = std::make_unique<Flower>(bonus);
  }

  Sector::get().add<SpecialRiser>(get_pos(), std::move(obj));
  SoundManager::current()->play("sounds/upgrade.wav", get_pos(), upgrade_sound_gain);
}

void
BonusBlock::drop_growup_bonus(Player* player, int type, const Direction& dir, bool& countdown)
{
  if (player->get_status().bonus[player->get_id()] == NO_BONUS)
  {
    Sector::get().add<GrowUp>(get_pos() + Vector(0, 32), dir);
  }
  else
  {
    Sector::get().add<PowerUp>(get_pos() + Vector(0, 32), type);
  }
  SoundManager::current()->play("sounds/upgrade.wav", get_pos(), upgrade_sound_gain);
  countdown = true;
}

void
BonusBlock::draw(DrawingContext& context)
{
  // do the regular drawing first
  Block::draw(context);
  // then Draw the light if on.
  if (m_sprite->get_action() == "on") {
    Vector pos = get_pos() + (m_col.m_bbox.get_size().as_vector() - Vector(static_cast<float>(m_lightsprite->get_width()),
                                                                   static_cast<float>(m_lightsprite->get_height()))) / 2.0f;
    context.light().draw_surface(m_lightsprite, pos, 10);
  }
}

BonusBlock::Content
BonusBlock::get_content_from_string(const std::string& contentstring) const
{
  if (contentstring == "coin") {
    return Content::COIN;
  } else if (contentstring == "firegrow") {
    return Content::FIREGROW;
  } else if (contentstring == "icegrow") {
    return Content::ICEGROW;
  } else if (contentstring == "airgrow") {
    return Content::AIRGROW;
  } else if (contentstring == "earthgrow") {
    return Content::EARTHGROW;
  } else if (contentstring == "star") {
    return Content::STAR;
  } else if (contentstring == "1up") {
    return Content::ONEUP;
  } else if (contentstring == "custom") {
    return Content::CUSTOM;
  } else if (contentstring == "script") { // use when bonusblock is to contain ONLY a script
    return Content::SCRIPT;
  } else if (contentstring == "light") {
    return Content::LIGHT;
  } else if (contentstring == "light-on") {
    return Content::LIGHT_ON;
  } else if (contentstring == "trampoline") {
    return Content::TRAMPOLINE;
  } else if (contentstring == "portabletrampoline") {
    return Content::PORTABLE_TRAMPOLINE;
  } else if (contentstring == "potion") {
    return Content::POTION;
  } else if (contentstring == "rock") {
    return Content::ROCK;
  } else if (contentstring == "rain") {
    return Content::RAIN;
  } else if (contentstring == "explode") {
    return Content::EXPLODE;
  } else {
    log_warning << "Invalid box contents '" << contentstring << "'" << std::endl;
    return Content::COIN;
  }
}

std::string
BonusBlock::contents_to_string(const BonusBlock::Content& content) const
{
  switch (m_contents)
  {
    case Content::COIN: return "coin";
    case Content::FIREGROW: return "firegrow";
    case Content::ICEGROW: return "icegrow";
    case Content::AIRGROW: return "airgrow";
    case Content::EARTHGROW: return "earthgrow";
    case Content::STAR: return "star";
    case Content::ONEUP: return "1up";
    case Content::CUSTOM: return "custom";
    case Content::SCRIPT: return "script";
    case Content::LIGHT: return "light";
    case Content::LIGHT_ON: return "light-on";
    case Content::TRAMPOLINE: return "trampoline";
    case Content::PORTABLE_TRAMPOLINE: return "portabletrampoline";
    case Content::POTION: return "potion";
    case Content::ROCK: return "rock";
    case Content::RAIN: return "rain";
    case Content::EXPLODE: return "explode";
    default: return "coin";
  }
}

void
BonusBlock::preload_contents(int d)
{
  switch (d)
  {
    case 6: // Light
    case 15: // Light (On)
      SoundManager::current()->preload("sounds/switch.ogg");
      m_lightsprite=Surface::from_file("/images/objects/lightmap_light/bonusblock_light.png");
      break;

    case 7:
      //object = new Trampoline(get_pos(), false); //needed if this is to be moved to custom
      break;

    case 8: // Trampoline
      m_object = std::make_unique<Trampoline>(get_pos(), true);
      break;

    case 9: // Rock
      m_object = std::make_unique<Rock>(get_pos(), "images/objects/rock/rock.sprite");
      break;

    case 12: // Red potion
      m_object = std::make_unique<PowerUp>(get_pos(), PowerUp::FLIP);
      break;

    default:
      break;
  }
}

/* EOF */
