//  SuperTux
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
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

#define _CRT_SECURE_NO_WARNINGS // Apparently required, according to Discord's examples

#include "sdk/discord.hpp"

extern "C" {
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "discord_rpc.h"
}

#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"

extern "C" {

  static void handleDiscordReady(const DiscordUser* connectedUser)
  {
      printf("\nDiscord: connected to user %s#%s - %s\n",
             connectedUser->username,
             connectedUser->discriminator,
             connectedUser->userId);
  }

  static void handleDiscordDisconnected(int errcode, const char* message)
  {
      printf("\nDiscord: disconnected (%d: %s)\n", errcode, message);
  }

  static void handleDiscordError(int errcode, const char* message)
  {
      printf("\nDiscord: error (%d: %s)\n", errcode, message);
  }

  static void handleDiscordJoin(const char* secret)
  {
      printf("\nDiscord: join (%s)\n", secret);
  }

  static void handleDiscordSpectate(const char* secret)
  {
      printf("\nDiscord: spectate (%s)\n", secret);
  }

  static void handleDiscordJoinRequest(const DiscordUser* request)
  {
      int response = -1;
      
      printf("\nDiscord: join request from %s#%s - %s\n",
             request->username,
             request->discriminator,
             request->userId);
      
      response = false ? DISCORD_REPLY_YES : DISCORD_REPLY_NO;
      
      if (response != -1) {
          Discord_Respond(request->userId, response);
      }
  }

} // extern "C"


DiscordIntegration* DiscordIntegration::singleton;

DiscordIntegration::DiscordIntegration() :
  m_status(IntegrationStatus::MAIN_MENU),
  m_level(),
  m_worldmap(),
  m_enabled(false)
{
}

DiscordIntegration*
DiscordIntegration::getSingleton()
{
  if (!singleton)
  {
    singleton = new DiscordIntegration();
  }
  return singleton;
}

void
DiscordIntegration::init()
{
  if (m_enabled || !g_config->enable_discord) return;

  DiscordEventHandlers handlers;
  memset(&handlers, 0, sizeof(handlers));
  handlers.ready = handleDiscordReady;
  handlers.disconnected = handleDiscordDisconnected;
  handlers.errored = handleDiscordError;
  handlers.joinGame = handleDiscordJoin;
  handlers.spectateGame = handleDiscordSpectate;
  handlers.joinRequest = handleDiscordJoinRequest;
  Discord_Initialize("733576109744062537", &handlers, 1, NULL);

  update_discord_presence();

  m_enabled = true;
}

void
DiscordIntegration::update()
{
  if (!m_enabled) return;

#ifdef DISCORD_DISABLE_IO_THREAD
  Discord_UpdateConnection();
#endif
  Discord_RunCallbacks();
}

void
DiscordIntegration::close()
{
  if (!m_enabled) return;

  Discord_ClearPresence();
  Discord_Shutdown();

  m_enabled = false;
}

void
DiscordIntegration::update_discord_presence()
{
  if (!m_enabled) return;

  DiscordRichPresence discordPresence;
  memset(&discordPresence, 0, sizeof(discordPresence));

  /*
   *  Possible keys :
   *   state
   *   details
   *   startTimestamp
   *   endTimestamp
   *   largeImageKey
   *   smallImageKey
   *   partyId
   *   partySize
   *   partyMax
   *   matchSecret
   *   joinSecret
   *   spectateSecret
   *   instance
   */

  char state_buffer[256];

  switch(m_status)
  {
  case IntegrationStatus::MAIN_MENU:
    discordPresence.state = "Main menu";
    break;

  case IntegrationStatus::PLAYING_WORLDMAP:
    sprintf(state_buffer, "In worldmap : %s", m_worldmap);
    discordPresence.state = state_buffer;
    discordPresence.smallImageKey = "play";
    break;

  case IntegrationStatus::PLAYING_LEVEL:
  case IntegrationStatus::PLAYING_LEVEL_FROM_WORLDMAP:
    sprintf(state_buffer, "Playing level : %s", m_level);
    discordPresence.state = state_buffer;
    discordPresence.details = m_worldmap;
    discordPresence.smallImageKey = "play";
    break;

  case IntegrationStatus::EDITING_WORLDMAP:
    if (!g_config->discord_hide_editor)
    {
      sprintf(state_buffer, "Editing worldmap : %s", m_worldmap);
      discordPresence.state = state_buffer;
    }
    else
    {
      discordPresence.state = "In editor";
    }
    discordPresence.smallImageKey = "edit";
    break;

  case IntegrationStatus::EDITING_LEVEL:
    if (!g_config->discord_hide_editor)
    {
      sprintf(state_buffer, "Editing level : %s", m_level);
      discordPresence.state = state_buffer;
      discordPresence.details = m_worldmap;
    }
    else
    {
      discordPresence.state = "In editor";
    }
    discordPresence.smallImageKey = "edit";
    break;

  case IntegrationStatus::TESTING_WORLDMAP:
    if (!g_config->discord_hide_editor)
    {
      sprintf(state_buffer, "Testing worldmap : %s", m_worldmap);
      discordPresence.state = state_buffer;
    }
    else
    {
      discordPresence.state = "In editor";
    }
    discordPresence.smallImageKey = "edit";
    break;

  case IntegrationStatus::TESTING_LEVEL:
  case IntegrationStatus::TESTING_LEVEL_FROM_WORLDMAP:
    if (!g_config->discord_hide_editor)
    {
      sprintf(state_buffer, "Testing level : %s", m_level);
      discordPresence.state = state_buffer;
      discordPresence.details = m_worldmap;
    }
    else
    {
      discordPresence.state = "In editor";
    }
    discordPresence.smallImageKey = "edit";
    break;

  default:
    break;
  }

  discordPresence.largeImageKey = "supertux_logo";
  discordPresence.startTimestamp = time(NULL); // TODO: Option to disable timers?
  Discord_UpdatePresence(&discordPresence);

}

void
DiscordIntegration::update_status(IntegrationStatus status)
{
  m_status = status;

  if (!m_enabled) return;
  update_discord_presence();
}

void
DiscordIntegration::update_worldmap(const char* worldmap)
{
  m_worldmap = new char[strlen(worldmap) + 1];
  strcpy(m_worldmap, worldmap);
}

void
DiscordIntegration::update_level(const char* level)
{
  printf("%s\n", level);
  m_level = new char[strlen(level) + 1];
  strcpy(m_level, level);
}

/* EOF */
