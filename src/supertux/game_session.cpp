//  SuperTux
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

#include "supertux/game_session.hpp"

#include <cfloat>

#include "audio/sound_manager.hpp"
#include "control/input_manager.hpp"
#include "editor/editor.hpp"
#include "gui/menu_manager.hpp"
#include "math/vector.hpp"
#include "network/host.hpp"
#include "network/server.hpp"
#include "object/camera.hpp"
#include "object/endsequence_fireworks.hpp"
#include "object/endsequence_walk.hpp"
#include "object/level_time.hpp"
#include "object/music_object.hpp"
#include "object/player.hpp"
#include "object/spawnpoint.hpp"
#include "sdk/integration.hpp"
#include "squirrel/squirrel_virtual_machine.hpp"
#include "supertux/constants.hpp"
#include "supertux/fadetoblack.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/game_network_protocol.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/level_parser.hpp"
#include "supertux/levelintro.hpp"
#include "supertux/levelset_screen.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/savegame.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "supertux/shrinkfade.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"
#include "util/file_system.hpp"
#include "video/compositor.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"
#include "worldmap/worldmap.hpp"

static const float SAFE_TIME = 1.0f;
static const int SHRINKFADE_LAYER = LAYER_LIGHTMAP - 1;
static const float TELEPORT_FADE_TIME = 1.0f;

GameSession::SpawnPoint::SpawnPoint(const std::string& sector_,
                                    const Vector& position_,
                                    bool is_checkpoint_) :
  sector(sector_),
  spawnpoint(),
  position(position_),
  is_checkpoint(is_checkpoint_)
{
}

GameSession::SpawnPoint::SpawnPoint(const std::string& sector_,
                                    const std::string& spawnpoint_,
                                    bool is_checkpoint_) :
  sector(sector_),
  spawnpoint(spawnpoint_),
  position(),
  is_checkpoint(is_checkpoint_)
{
}

GameSession::SpawnPoint::SpawnPoint(const std::string& data) :
  sector(),
  spawnpoint(),
  position(),
  is_checkpoint()
{
  auto doc = ReaderDocument::from_string(data, "spawnpoint");
  auto root = doc.get_root();
  if (root.get_name() != "supertux-spawnpoint")
    throw std::runtime_error("Couldn't parse spawnpoint: Data is not 'supertux-spawnpoint'.");

  auto reader = root.get_mapping();

  reader.get("sector", sector);
  if (!reader.get("spawnpoint", spawnpoint))
  {
    reader.get("position_x", position.x);
    reader.get("position_y", position.y);
  }
  reader.get("is_checkpoint", is_checkpoint);
}

std::string
GameSession::SpawnPoint::serialize() const
{
  std::ostringstream stream;
  Writer writer(stream);

  writer.start_list("supertux-spawnpoint");
  {
    writer.write("sector", sector);
    if (spawnpoint.empty())
    {
      writer.write("position_x", position.x);
      writer.write("position_y", position.y);
    }
    else
    {
      writer.write("spawnpoint", spawnpoint);
    }
    writer.write("is_checkpoint", is_checkpoint);
  }
  writer.end_list("supertux-spawnpoint");

  return stream.str();
}


GameSession::SpawnRequest::SpawnRequest() :
  user(nullptr),
  all_users(true),
  sector(),
  spawnpoint(),
  fade_type(ScreenFade::FadeType::NONE),
  fade_timer(new Timer()),
  with_invincibility(false)
{
}


GameSession::GameSession(const std::string& levelfile, Savegame& savegame, Statistics* statistics,
                         bool preserve_music, const std::optional<SpawnPoint>& spawnpoint,
                         network::Host* host) :
  reset_button(false),
  reset_checkpoint_button(false),
  m_prevent_death(false),
  m_level(),
  m_statistics_backdrop(Surface::from_file("images/engine/menu/score-backdrop.png")),
  m_data_table(SquirrelVirtualMachine::current()->get_vm().findTable("Level").getOrCreateTable("data")),
  m_currentsector(nullptr),
  m_game_pause(false),
  m_speed_before_pause(ScreenManager::current()->get_speed()),
  m_levelfile(levelfile),
  m_network_host(host),
  m_spawnpoints(),
  m_activated_checkpoint(),
  m_spawn_requests(),
  m_best_level_statistics(statistics),
  m_savegame(savegame),
  m_play_time(0),
  m_levelintro(),
  m_levelintro_shown(false),
  m_active(false),
  m_end_seq_started(false),
  m_pause_target_timer(false),
  m_current_cutscene_text(),
  m_endsequence_timer()
{
  if (spawnpoint)
    m_spawnpoints.push_back(*spawnpoint);
  else
    set_start_point(DEFAULT_SECTOR_NAME, DEFAULT_SPAWNPOINT_NAME);

  m_data_table.clear();

  if (restart_level(false, preserve_music) != 0)
    throw std::runtime_error ("Initializing the level failed.");
}

void
GameSession::spawn_local_player(int id, const GameServerUser* target_user, int target_id)
{
  assert(id > 0);
  if (m_savegame.is_title_screen())
    return;

  m_savegame.get_player_status().add_local_player(id);

  // If we are a client and no target user for the player was provided,
  // notify the user of this player spawning and request the target user and ID for it.
  if (!m_levelintro && m_network_host && !m_network_host->is_server() && target_id < 0)
  {
    m_network_host->send_request(GameManager::current()->get_server_peer(),
                                 std::make_unique<network::Request>(
                                   std::make_unique<network::StagedPacket>(GameNetworkProtocol::OP_PLAYER_SPAWN, std::to_string(id), 2.f),
                                   5.f));
    return;
  }

  Player& player = m_currentsector->add<Player>(m_savegame.get_player_status(), id);
  m_currentsector->flush_game_objects();

  if (m_levelintro)
  {
    m_levelintro->push_player(&player);

    // Activate on either the spawnpoint (if set), or the spawn position.
    if (m_spawnpoints.front().spawnpoint.empty())
      m_currentsector->spawn_players(m_spawnpoints.front().position);
    else
      m_currentsector->spawn_players(m_spawnpoints.front().spawnpoint);
  }
  else
  {
    player.multiplayer_prepare_spawn(target_user == &*GameManager::current()->get_self_user() ? nullptr : target_user, target_id);

    if (!player.get_target())
      return;

    if (m_network_host && m_network_host->is_server())
    {
      const Player* target = m_currentsector->get_object_by_uid<Player>(*player.get_target());
      const GameServerUser* target_user = target->get_remote_user();
      if (!target_user)
        target_user = &*GameManager::current()->get_self_user();

      network::StagedPacket packet(GameNetworkProtocol::OP_PLAYER_SPAWN, {
           GameManager::current()->get_self_user()->username,
           std::to_string(id),
           target_user->username,
           std::to_string(target->get_id())
        });
      m_network_host->broadcast_packet(packet, true);
    }
  }
}

bool
GameSession::despawn_local_player(int id)
{
  assert(id > 0);
  if (m_savegame.is_title_screen())
    return false;

  for (Player* player : m_level->get_players())
  {
    if (!player->get_remote_user() && player->get_id() == id)
    {
      if (m_levelintro)
        m_levelintro->pop_player(player);

      if (m_network_host)
      {
        network::StagedPacket packet(GameNetworkProtocol::OP_PLAYER_DESPAWN, std::to_string(id));
        m_network_host->broadcast_packet(packet, true);
      }

      player->remove_me();
      return true;
    }
  }
  return false;
}

std::pair<const GameServerUser*, int>
GameSession::spawn_remote_player(const GameServerUser& user, int id,
                                 const GameServerUser* target_user, int target_id)
{
  if (m_savegame.is_title_screen())
    return { nullptr, -1 };

  m_savegame.get_player_status().add_remote_player(user.username, id);

  Player& player = m_currentsector->add<Player>(m_savegame.get_player_status(), id, &user);
  m_currentsector->flush_game_objects();

  if (m_levelintro)
  {
    m_levelintro->push_player(&player);

    // Activate on either the spawnpoint (if set), or the spawn position.
    if (m_spawnpoints.front().spawnpoint.empty())
      m_currentsector->spawn_players(m_spawnpoints.front().position);
    else
      m_currentsector->spawn_players(m_spawnpoints.front().spawnpoint);
  }
  else
  {
    player.multiplayer_prepare_spawn(target_user == &*GameManager::current()->get_self_user() ? nullptr : target_user, target_id);

    if (!player.get_target())
      return { nullptr, -1 };

    const Player* target = m_currentsector->get_object_by_uid<Player>(*player.get_target());
    const GameServerUser* target_user = target->get_remote_user();
    if (!target_user)
      target_user = &*GameManager::current()->get_self_user();

    if (m_network_host && m_network_host->is_server())
    {
      network::StagedPacket packet(GameNetworkProtocol::OP_PLAYER_SPAWN, {
          user.username,
          std::to_string(id),
          target_user->username,
          std::to_string(target->get_id())
        });

      network::Server* server = static_cast<network::Server*>(m_network_host);
      m_network_host->broadcast_packet(packet, true, server->get_peer_from_user(user));
    }

    return { target_user, target->get_id() };
  }

  return { nullptr, -1 };
}

bool
GameSession::despawn_remote_player(const GameServerUser& user, int id)
{
  if (m_savegame.is_title_screen())
    return false;

  for (Player* player : m_level->get_players())
  {
    if (player->get_remote_user() == &user && player->get_id() == id)
    {
      if (m_levelintro)
        m_levelintro->pop_player(player);

      player->remove_me();
      return true;
    }
  }
  return false;
}

void
GameSession::reset_level()
{
  m_savegame.get_player_status().restore_state();

  for (const auto& p : m_currentsector->get_players())
    p->set_bonus(p->get_status().bonus);

  clear_respawn_points();
  m_activated_checkpoint = nullptr;
  m_pause_target_timer = false;

  m_data_table.clear();
}

int
GameSession::restart_level(bool after_death, bool preserve_music)
{
  m_savegame.get_player_status().save_state();

  // Needed for the title screen apparently.
  /*
  if (m_currentsector)
  {
    try
    {
      for (const auto& p : m_currentsector->get_players())
      {
        p->set_bonus(m_boni_at_start.at(p->get_id()), false, false);
        m_boni_at_start[p->get_id()] = currentStatus.bonus[p->get_id()];
      }
    }
    catch (const std::out_of_range&)
    {
    }
  }
  */

  m_game_pause = false;
  m_endsequence_timer.stop();

  InputManager::current()->reset();

  m_currentsector = nullptr;

  if (!(m_network_host && !m_network_host->is_server()))
  {
    const std::string base_dir = FileSystem::dirname(m_levelfile);
    if (base_dir == "./") {
      m_levelfile = FileSystem::basename(m_levelfile);
    }
  }

  try {
    if (m_network_host && !m_network_host->is_server())
    {
      std::istringstream level_stream(m_levelfile);

      GameObject::s_read_uid = true;
      m_level = LevelParser::from_stream(level_stream, "remote-level", false, false);
      GameObject::s_read_uid = false;
    }
    else
    {
      m_level = LevelParser::from_file(m_levelfile, false, false);
    }

    /* Determine the spawnpoint to spawn/respawn Tux to. */
    const GameSession::SpawnPoint* spawnpoint = nullptr;
    if (m_activated_checkpoint && reset_checkpoint_button) // Checkpoint is activated and respawn from it is requested.
    {
      reset_checkpoint_button = false;

      // Attempt to find the last checkpoint's spawnpoint.
      for (int i = static_cast<int>(m_spawnpoints.size()) - 1; i >= 0; i--)
      {
        if (m_spawnpoints.at(i).is_checkpoint) // Checkpoint found.
        {
          spawnpoint = &m_spawnpoints.at(i);

          // Remove any spawnpoints after the checkpoint one.
          if (i < static_cast<int>(m_spawnpoints.size()) - 1)
            m_spawnpoints.erase(m_spawnpoints.begin() + i + 1, m_spawnpoints.end());

          break;
        }
      }

      if (!spawnpoint) // Spawnpoint has not been found.
      {
        throw std::runtime_error("Cannot find the position of the last activated checkpoint.");
      }
    }
    else if (after_death && m_spawnpoints.size() > 1) // Respawn from the last respawn position, because Tux is respawning.
    {
      spawnpoint = &get_last_spawnpoint();
    }
    else // Respawn from the start position.
    {
      spawnpoint = &m_spawnpoints.front();

      m_play_time = 0; // Reset play time.
      m_data_table.clear();
    }

    /* Perform the respawn from the chosen spawnpoint. */
    // Load the spawn sector.
    m_currentsector = m_level->get_sector(spawnpoint->sector);
    if (!m_currentsector)
      throw std::runtime_error("Couldn't find sector '" + spawnpoint->sector + "' to spawn/respawn Tux.");

    // Spawn players on either the spawnpoint (if set), or the spawn position.
    if (spawnpoint->spawnpoint.empty())
      m_currentsector->spawn_players(spawnpoint->position);
    else
      m_currentsector->spawn_players(spawnpoint->spawnpoint);

    m_currentsector->run_init_script();
  }
  catch (std::exception& e) {
    log_fatal << "Couldn't start level: " << e.what() << std::endl;
    ScreenManager::current()->pop_screen();
    return (-1);
  }

  if (m_network_host && m_network_host->is_server())
  {
    // Request all clients to start a GameSession with this level to join the game.
    GameObject::s_save_uid = true;
    network::StagedPacket packet(GameNetworkProtocol::OP_GAME_JOIN,
      {
        m_savegame.get_player_status().write(false),
        m_level->save(),
        m_spawnpoints.front().serialize()
      });
    GameObject::s_save_uid = false;

    m_network_host->broadcast_packet(packet, true);
  }

  if (m_levelintro_shown)
  {
    const Vector shrinkpos = get_fade_point();
    ScreenManager::current()->set_screen_fade(std::make_unique<ShrinkFade>(shrinkpos, TELEPORT_FADE_TIME, SHRINKFADE_LAYER,  ShrinkFade::FADEIN));
  }

  if (!preserve_music)
  {
    auto& music_object = m_currentsector->get_singleton_by_type<MusicObject>();
    if (after_death == true) {
      music_object.resume_music();
    } else {
      SoundManager::current()->stop_music();
      music_object.play_music(LEVEL_MUSIC);
    }
  }

  auto level_times = m_currentsector->get_objects_by_type<LevelTime>();
  auto it = level_times.begin();

  while (it != level_times.end())
  {
    it->set_time(it->get_time() - m_play_time);
    it++;
  }

  return (0);
}

void
GameSession::on_escape_press(bool force_quick_respawn)
{
  auto players = m_level->get_players();

  bool has_alive_players = false;
  for (const Player* player : players)
  {
    if (player->is_alive())
    {
      has_alive_players = true;
      break;
    }
  }

  EndSequence* sequence = m_currentsector->try_get_singleton_by_type<EndSequence>();
  if ((!has_alive_players && (m_play_time > 2.0f || force_quick_respawn)) || sequence)
  {
    // Let the timers run out, we fast-forward them to force past a sequence.
    if (sequence)
    {
      if (sequence->is_running())
      {
        for (const auto& sector : m_level->get_sectors())
          sector->get_singleton_by_type<EndSequence>().stop();
      }
      else
      {
        m_endsequence_timer.start(FLT_EPSILON);
      }
    }

    for (auto* player : players)
      player->m_dying_timer.start(FLT_EPSILON);

    return;   // Don't let the player open the menu, when Tux is dying.
  }

  if (m_level->m_is_in_cutscene && !m_level->m_skip_cutscene)
  {
    m_level->m_skip_cutscene = true;
    return;
  }

  if (m_level->m_suppress_pause_menu && ScreenManager::current()->has_pending_fadeout())
    return;

  if (!m_level->m_suppress_pause_menu) {
    toggle_pause();
  } else {
    abort_level();
  }
}

Vector
GameSession::get_fade_point(const Vector& position) const
{
  Vector fade_point(0.0f, 0.0f);

  if (position.x != 0.0f && position.y != 0.0f)
  {
    fade_point = position;
  }
  else
  {
    if (m_level->m_is_in_cutscene || m_currentsector->get_camera().get_mode() == Camera::Mode::MANUAL)
    {
      fade_point = m_currentsector->get_camera().get_center();
    }
    else
    {
      // Get "middle" of all alive players
      Vector average_position(0.0f, 0.0f);
      size_t alive_players = 0U;

      for (const auto* player : m_currentsector->get_players())
      {
        if (player->is_alive())
        {
          average_position += player->get_bbox().get_middle();
          alive_players++;
        }
      }

      if (alive_players > 0U)
      {
        fade_point = average_position / alive_players;
      }
      else
      {
        fade_point = m_currentsector->get_camera().get_center();
      }
    }
  }

  const Camera& camera = m_currentsector->get_camera();

  return (fade_point - camera.get_translation()) * camera.get_current_scale();
}

void
GameSession::toggle_pause()
{
  // Pause.
  if (!m_game_pause && !MenuManager::instance().is_active())
  {
    m_speed_before_pause = ScreenManager::current()->get_speed();
    MenuManager::instance().set_menu(MenuStorage::GAME_MENU);
    if (!m_network_host)
    {
      ScreenManager::current()->set_speed(0);
      SoundManager::current()->pause_sounds();
      m_currentsector->stop_looping_sounds();
      SoundManager::current()->pause_music();
    }
    m_game_pause = true;
  }

  // Unpause is done in update() after the menu is processed.
}

void
GameSession::abort_level()
{
  MenuManager::instance().clear_menu_stack();
  ScreenManager::current()->pop_screen();

  m_savegame.get_player_status().restore_state();

  for (const auto& p : m_currentsector->get_players())
    p->set_bonus(p->get_status().bonus);

  SoundManager::current()->stop_sounds();
}

bool
GameSession::is_active() const
{
  if (m_game_pause || !m_active)
    return false;

  const EndSequence* sequence = m_currentsector->try_get_singleton_by_type<EndSequence>();
  if (sequence && sequence->is_running())
    return false;

  return true;
}

void
GameSession::check_end_conditions()
{
  bool all_dead = true;
  for (const auto* p : m_currentsector->get_players())
    if (!(all_dead &= p->is_dead()))
      break;

  bool all_dead_or_winning = true;
  for (const auto* p : m_currentsector->get_players())
    if (!(all_dead_or_winning &= (!p->is_active())))
      break;

  /* End of level? */
  const EndSequence* sequence = m_currentsector->try_get_singleton_by_type<EndSequence>();
  if (m_endsequence_timer.check() || (all_dead_or_winning && sequence && m_endsequence_timer.get_period() > 0.f))
  {
    m_endsequence_timer.stop();
    start_sequence(nullptr, Sequence::SEQ_ENDSEQUENCE);
  }
  else if (sequence && sequence->is_done())
  {
    finish(true);
  }
  else if (!sequence && all_dead)
  {
    restart_level(true);
  }
}

void
GameSession::draw(Compositor& compositor)
{
  auto& context = compositor.make_context();

  if (m_game_pause) {
    context.set_time_offset(0.0f);
  }

  m_currentsector->draw(context);
  drawstatus(context);

  if (m_game_pause)
    draw_pause(context);
}

void
GameSession::draw_pause(DrawingContext& context)
{
  context.color().draw_filled_rect(
    Rectf(context.get_rect()),
    Color(0.0f, 0.0f, 0.0f, 0.25f),
    LAYER_FOREGROUND1);
}

void
GameSession::setup()
{
  assert(m_currentsector);

  m_currentsector->spawn_players(m_currentsector->get_players()[0]->get_pos(), nullptr, true);
  m_currentsector->get_singleton_by_type<MusicObject>().play_music(LEVEL_MUSIC);

  for (const auto& sector : m_level->get_sectors())
    sector->expose();

  //int total_stats_to_be_collected = m_level->m_stats.m_total_coins + m_level->m_stats.m_total_badguys + m_level->m_stats.m_total_secrets;
  if (!m_levelintro_shown)
  {
    m_levelintro_shown = true;
    m_active = false;

    auto levelintro = std::make_unique<LevelIntro>(*m_level, m_best_level_statistics,
                                                   !(m_network_host && !m_network_host->is_server()));
    m_levelintro = levelintro.get();

    ScreenManager::current()->push_screen(std::move(levelintro),
                                          std::make_unique<FadeToBlack>(FadeToBlack::FADEIN, TELEPORT_FADE_TIME));
  }
  else
  {
    m_levelintro = nullptr;

    if (m_network_host && m_network_host->is_server())
    {
      network::StagedPacket packet(GameNetworkProtocol::OP_GAME_START, "");
      m_network_host->broadcast_packet(packet, true);
    }

    const Vector shrinkpos = get_fade_point();
    ScreenManager::current()->set_screen_fade(std::make_unique<ShrinkFade>(shrinkpos, TELEPORT_FADE_TIME, SHRINKFADE_LAYER, ShrinkFade::FADEIN));
  }


  m_end_seq_started = false;
}

void
GameSession::leave()
{
  for (const auto& sector : m_level->get_sectors())
    sector->unexpose();

  m_data_table.clear();
}

void
GameSession::update(float dt_sec, const Controller& controller)
{
  // Set active flag.
  if (!m_active)
  {
    m_active = true;
  }
  // Handle controller.

  if (controller.pressed_any(Control::ESCAPE, Control::START))
  {
    on_escape_press(controller.hold(Control::LEFT)
      || controller.hold(Control::RIGHT));
  }

  if (controller.pressed(Control::CHEAT_MENU) &&
      (g_config->developer_mode || (Editor::current() && Editor::current()->is_testing_level()))
     )
  {
    if (!MenuManager::instance().is_active())
    {
      toggle_pause();
      MenuManager::instance().set_menu(MenuStorage::CHEAT_MENU);
    }
  }

  if (controller.pressed(Control::DEBUG_MENU) && g_config->developer_mode)
  {
    if (!MenuManager::instance().is_active())
    {
      toggle_pause();
      MenuManager::instance().set_menu(MenuStorage::DEBUG_MENU);
    }
  }

  if (m_network_host)
  {
    // Notify the server of controller updates.
    for (int user = 0; user < InputManager::current()->get_num_users(); user++)
    {
      const Controller& user_controller = InputManager::current()->get_controller(user);
      if (user_controller.has_pressed_controls())
      {
        network::StagedPacket packet(GameNetworkProtocol::OP_CONTROLLER_UPDATE, std::to_string(user));
        user_controller.push_packet_data(packet);
        m_network_host->broadcast_packet(packet, false);
      }
    }
  }

  // Animate the full-completion stats stuff - do this even when the game isn't paused (that's a
  // design choice, if you prefer it not to animate when paused, add `if (!m_game_pause)`).
  m_level->m_stats.update_timers(dt_sec);

  // Unpause the game if the menu has been closed.
  if (m_game_pause && !MenuManager::instance().is_active()) {
    ScreenManager::current()->set_speed(m_speed_before_pause);
    SoundManager::current()->resume_music();
    SoundManager::current()->resume_sounds();
    assert(m_currentsector != nullptr);
    m_currentsector->play_looping_sounds();
    m_game_pause = false;
  }

  check_end_conditions();

  // Requests to respawn in new sector?
  auto it_spawn_requests = m_spawn_requests.begin();
  while (it_spawn_requests != m_spawn_requests.end())
  {
    SpawnRequest& request = *it_spawn_requests;
    if (request.fade_type != ScreenFade::FadeType::NONE && !request.fade_timer->check())
    {
      it_spawn_requests++;
      continue;
    }
    const bool request_local = request.all_users || !request.user;

    auto sector = m_level->get_sector(request.sector);
    if (sector == nullptr)
    {
      log_warning << "Sector '" << request.sector << "' not found" << std::endl;
      sector = m_level->get_sector(m_spawnpoints.at(0).sector); // Assign start sector.
    }

    const bool sector_has_players = sector->get_object_count<Player>() > 0;
    sector->spawn_players(request.spawnpoint, request.user, request.all_users);
    if (!sector_has_players)
      sector->run_init_script();

    if (request_local)
    {
      assert(m_currentsector);

      // Start the new sector's music only if it's different from the current one.
      auto& music_obj = sector->get_singleton_by_type<MusicObject>();
      if (m_currentsector->get_singleton_by_type<MusicObject>().get_music() != music_obj.get_music())
        music_obj.play_music(LEVEL_MUSIC);

      m_currentsector->stop_looping_sounds();
      m_currentsector = sector;
      m_currentsector->play_looping_sounds();

      switch (request.fade_type)
      {
        case ScreenFade::FadeType::FADE:
        {
          ScreenManager::current()->set_screen_fade(std::make_unique<FadeToBlack>(FadeToBlack::FADEIN, TELEPORT_FADE_TIME));
          break;
        }
        case ScreenFade::FadeType::CIRCLE:
        {
          const Vector spawn_point_position = sector->get_spawn_point_position(request.spawnpoint);
          const Vector shrinkpos = get_fade_point(spawn_point_position);

          ScreenManager::current()->set_screen_fade(std::make_unique<ShrinkFade>(shrinkpos, TELEPORT_FADE_TIME, SHRINKFADE_LAYER, ShrinkFade::FADEIN));
          break;
        }
        default:
          break;
      }
    }

    for (Player* player : m_level->get_players())
    {
      if (!request.all_users && player->get_remote_user() != request.user)
        continue;

      // Give back control to the player
      player->activate();

      if (request.with_invincibility)
      {
        // Make all players temporarily safe after spawning
        player->make_temporarily_safe(SAFE_TIME);
      }
    }

    it_spawn_requests = m_spawn_requests.erase(it_spawn_requests);
  }

  assert(m_currentsector);

  // Update the world state and all objects in the world.
  const EndSequence* sequence = m_currentsector->try_get_singleton_by_type<EndSequence>();
  if (!m_game_pause || m_network_host)
  {
    // Update the world.
    if (!sequence || !sequence->is_running())
    {
      if (!m_level->m_is_in_cutscene && !m_pause_target_timer)
      {
        m_play_time += dt_sec;
        m_level->m_stats.finish(m_play_time);
      }

      for (const auto& sector : m_level->get_sectors())
      {
        if (sector->get_object_count<Player>() > 0)
          sector->update(dt_sec);
      }
    }
    else
    {
      bool are_all_stopped = true;

      for (const auto& sector : m_level->get_sectors())
      {
        auto& sector_sequence = sector->get_singleton_by_type<EndSequence>();
        for (const auto& player : sector->get_objects_by_type<Player>())
        {
          if (!sector_sequence.is_tux_stopped(player.get_uid()) && player.get_ending_direction() != 0)
          {
            are_all_stopped = false;
            break;
          }
        }
      }

      if (!are_all_stopped)
      {
        for (const auto& sector : m_level->get_sectors())
        {
          if (sector->get_object_count<Player>() > 0)
            sector->update(dt_sec);
        }
      }
      else
      {
        for (const auto& sector : m_level->get_sectors())
        {
          sector->get_singleton_by_type<EndSequence>().update(dt_sec);
          for (auto& player : sector->get_objects_by_type<Player>())
            player.update(dt_sec);
        }
      }
    }
  }

  // Update sounds.
  SoundManager::current()->set_listener_position(m_currentsector->get_camera().get_center());

  /* Handle music: */
  if (sequence && sequence->is_running())
    return;

  bool invincible_timer_started = false;
  float max_invincible_timer_left = 0.f;

  for (const auto* p : m_currentsector->get_players())
  {
    invincible_timer_started |= (p->m_invincible_timer.started() && !p->is_winning());
    max_invincible_timer_left = std::max(max_invincible_timer_left, p->m_invincible_timer.get_timeleft());
  }

  auto& music_object = m_currentsector->get_singleton_by_type<MusicObject>();
  if (invincible_timer_started) {
    if (max_invincible_timer_left <= TUX_INVINCIBLE_TIME_WARNING) {
      if (music_object.get_music_type() != HERRING_WARNING_MUSIC)
        music_object.play_music(HERRING_WARNING_MUSIC);
    } else {
      if (music_object.get_music_type() != HERRING_MUSIC)
        music_object.play_music(HERRING_MUSIC);
    }
  } else if (music_object.get_music_type() != LEVEL_MUSIC) {
    music_object.play_music(LEVEL_MUSIC);
  }
  if (reset_button) {
    reset_button = false;
    reset_level();
    restart_level();
  } else if(reset_checkpoint_button) { // TODO: Remote player/multi-sector support
    for (auto* p : m_currentsector->get_players())
      p->kill(true);
  }
}

IntegrationStatus
GameSession::get_status() const
{
  IntegrationStatus status;
  status.m_details.push_back(Editor::current() ? "Testing" : "Playing");
  if (!Editor::current() || !g_config->hide_editor_levelnames)
  {
    const std::string label = get_current_level().is_worldmap() ? "In worldmap: " : "In level: ";
    status.m_details.push_back(label + get_current_level().get_name());
  }
  return status;
}

void
GameSession::finish(bool win)
{
  if (m_end_seq_started)
    return;
  m_end_seq_started = true;

  using namespace worldmap;

  if (win) {
    if (WorldMapSector::current())
    {
      WorldMapSector::current()->finished_level(m_level.get());
    }

    if (LevelsetScreen::current())
    {
      LevelsetScreen::current()->finished_level(win);
    }
  }

  ScreenManager::current()->pop_screen();
}

void
GameSession::respawn(const std::string& sector, const std::string& spawnpoint,
                     const GameServerUser* user, bool all_users)
{
  SpawnRequest request;
  request.user = user;
  request.all_users = all_users;
  request.sector = sector;
  request.spawnpoint = spawnpoint;

  m_spawn_requests.push_back(std::move(request));
}

void
GameSession::respawn_with_fade(const std::string& sector,
                               const std::string& spawnpoint,
                               const ScreenFade::FadeType fade_type,
                               const Vector& fade_point,
                               const bool make_invincible,
                               const GameServerUser* user, bool all_users)
{
  respawn(sector, spawnpoint, user, all_users);
  SpawnRequest& request = m_spawn_requests.back();

  request.fade_type = fade_type;
  request.with_invincibility = make_invincible;

  bool transition_takes_time = false;
  switch (fade_type)
  {
    case ScreenFade::FadeType::FADE:
    {
      if (all_users || !user)
      {
        ScreenManager::current()->set_screen_fade(std::make_unique<FadeToBlack>(FadeToBlack::FADEOUT, TELEPORT_FADE_TIME));
      }
      transition_takes_time = true;
      break;
    }
    case ScreenFade::FadeType::CIRCLE:
    {
      if (all_users || !user)
      {
        const Vector shrinkpos = get_fade_point(fade_point);
        ScreenManager::current()->set_screen_fade(std::make_unique<ShrinkFade>(shrinkpos, TELEPORT_FADE_TIME, SHRINKFADE_LAYER, ShrinkFade::FADEOUT));
      }
      transition_takes_time = true;
      break;
    }
    default:
      break;
  }

  if (transition_takes_time)
  {
    request.fade_timer->start(TELEPORT_FADE_TIME);

    // Make all players safe during the fadeout transition
    for (Player* player : m_level->get_players())
    {
      if (all_users || player->get_remote_user() == user)
        player->make_temporarily_safe(TELEPORT_FADE_TIME);
    }
  }
}

void
GameSession::set_start_point(const std::string& sector,
                             const std::string& spawnpoint)
{
  if (!m_spawnpoints.empty()) m_spawnpoints.erase(m_spawnpoints.begin());
  m_spawnpoints.insert(m_spawnpoints.begin(), SpawnPoint(sector, spawnpoint));
}

void
GameSession::set_start_pos(const std::string& sector, const Vector& pos)
{
  if (!m_spawnpoints.empty()) m_spawnpoints.erase(m_spawnpoints.begin());
  m_spawnpoints.insert(m_spawnpoints.begin(), SpawnPoint(sector, pos));
}

void
GameSession::set_respawn_point(const std::string& sector,
                               const std::string& spawnpoint)
{
  m_spawnpoints.push_back(SpawnPoint(sector, spawnpoint));
}

void
GameSession::set_respawn_pos(const std::string& sector, const Vector& pos)
{
  m_spawnpoints.push_back(SpawnPoint(sector, pos));
}

void
GameSession::clear_respawn_points()
{
  // Delete all respawn points (all, other than the start one).
  if (m_spawnpoints.size() > 1)
    m_spawnpoints.erase(m_spawnpoints.begin() + 1, m_spawnpoints.end());
}

const GameSession::SpawnPoint&
GameSession::get_last_spawnpoint() const
{
  return m_spawnpoints.back();
}

void
GameSession::set_checkpoint_pos(const std::string& sector, const Vector& pos)
{
  m_spawnpoints.push_back({ sector, pos, true });
  m_activated_checkpoint = &m_spawnpoints.back();
}

const GameSession::SpawnPoint*
GameSession::get_active_checkpoint_spawnpoint() const
{
  return m_activated_checkpoint;
}

bool
GameSession::has_active_sequence() const
{
  const EndSequence* sequence = m_currentsector->try_get_singleton_by_type<EndSequence>();
  return sequence && sequence->is_running();
}

void
GameSession::start_sequence(Player* caller, Sequence seq, const SequenceData* data)
{
  // Handle special "stoptux" sequence.
  if (seq == SEQ_STOPTUX)
  {
    // TODO: This probably doesn't work with multiplayer
    if (caller)
    {
      EndSequence* sequence = caller->get_parent()->try_get_singleton_by_type<EndSequence>();
      if (!sequence)
      {
        log_warning << "Final target reached without an active end sequence" << std::endl;
        start_sequence(caller, SEQ_ENDSEQUENCE);
        sequence = &caller->get_parent()->get_singleton_by_type<EndSequence>();
      }
      sequence->stop_tux(caller->get_uid());
    }
    else
    {
      for (const auto& sector : m_level->get_sectors())
      {
        auto& sequence = sector->get_singleton_by_type<EndSequence>();
        for (const Player* player : sector->get_players())
          sequence.stop_tux(player->get_uid());
      }
    }
    return;
  }

  if (caller)
    caller->set_winning();

  // Abort if a sequence is already playing.
  const EndSequence* sequence = m_currentsector->try_get_singleton_by_type<EndSequence>();
  if (sequence && sequence->is_running())
    return;

  // Set the sequence to prepare it.
  if (!sequence)
  {
    if (seq == SEQ_ENDSEQUENCE)
    {
      for (const auto& sector : m_level->get_sectors())
      {
        sector->add<EndSequenceWalk>();
        sector->flush_game_objects();
      }
    }
    else if (seq == SEQ_FIREWORKS)
    {
      for (const auto& sector : m_level->get_sectors())
      {
        sector->add<EndSequenceFireworks>();
        sector->flush_game_objects();
      }
    }
    else
    {
      log_warning << "Unknown sequence '" << static_cast<int>(seq) << "'. Ignoring." << std::endl;
      return;
    }
  }

  if (caller)
  {
    caller->set_ending_direction((caller->get_physic().get_velocity_x() < 0) ? -1 : 1);
    caller->set_controller(caller->get_parent()->get_singleton_by_type<EndSequence>().get_controller(caller->get_uid()));
    caller->set_speedlimit(230); // MAX_WALK_XM
  }

  bool has_remaining_players = false;
  for (const Player* player : m_level->get_players())
  {
    if (player->is_active())
    {
      has_remaining_players = true;
      break;
    }
  }

  // Don't play the prepared sequence if there are more players that are still playing.
  if (has_remaining_players)
  {
    // Only start the forced end sequence timer if not in a network game.
    if (!m_network_host && !m_endsequence_timer.started())
      m_endsequence_timer.start(10.f);

    return;
  }

  m_endsequence_timer.stop();

  if (const auto& worldmap_sector = worldmap::WorldMapSector::current())
  {
    if (data != nullptr)
    {
      if (!data->fade_tilemap.empty())
      {
        worldmap_sector->set_initial_fade_tilemap(data->fade_tilemap, data->fade_type);
      }
      if (!data->spawnpoint.empty())
      {
        worldmap_sector->get_worldmap().set_initial_spawnpoint(data->spawnpoint);
      }
    }
  }

  /* Slow down the game for end-sequence. */
  ScreenManager::current()->set_speed(0.5f);
  SoundManager::current()->play_music("music/misc/leveldone.ogg", false);

  for (const auto& sector : m_level->get_sectors())
  {
    auto& sector_sequence = sector->get_singleton_by_type<EndSequence>();
    for (Player* player : sector->get_players())
    {
      player->set_winning();
      player->set_controller(sector_sequence.get_controller(player->get_uid()));
      player->set_speedlimit(230); // MAX_WALK_XM.
    }
    sector_sequence.start();

    // Stop all clocks.
    for (LevelTime& lt : sector->get_objects_by_type<LevelTime>())
      lt.stop();
  }
}

void
GameSession::set_target_timer_paused(bool paused)
{
  m_pause_target_timer = paused;
  for (LevelTime& lt : m_currentsector->get_objects_by_type<LevelTime>())
  {
    if(paused)
      lt.stop();
    else
      lt.start();
  }
}

/* (Status): */
void
GameSession::drawstatus(DrawingContext& context)
{
  // Draw level stats while end_sequence is running.
  const EndSequence* sequence = m_currentsector->try_get_singleton_by_type<EndSequence>();
  if (sequence && sequence->is_running())
    m_level->m_stats.draw_endseq_panel(context, m_best_level_statistics, m_statistics_backdrop, m_level->m_target_time);

  m_level->m_stats.draw_ingame_stats(context, m_game_pause);
}

/* EOF */
