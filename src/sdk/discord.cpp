//  SuperTux
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
//  Copyright (C) 2025 J. Elsing <Johannes.Elsing@gmx.de>
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

#include "config.h"

#ifdef ENABLE_DISCORD

#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <memory>
#include <thread>

#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "util/log.hpp"

#include "discord.hpp"

#define DISCORDPP_IMPLEMENTATION
#include "discordpp.h"

const uint64_t APPLICATION_ID = 1393721527370780742;
DiscordIntegration* DiscordIntegration::driver = nullptr;

DiscordIntegration::DiscordIntegration() : m_enabled(false), m_running(true) {}

DiscordIntegration::~DiscordIntegration() {
  m_running = false;
}

DiscordIntegration* DiscordIntegration::getDriver() {
  if (!driver) {
    driver = new DiscordIntegration();
  }
  return driver;
}

void DiscordIntegration::init()
{
  if (m_enabled) return;
  m_enabled = true;
  m_running = true;

  m_client = std::make_shared<discordpp::Client>();

  m_client->AddLogCallback([](auto message, auto severity) {
    std::cout << "[" << EnumToString(severity) << "] " << message << std::endl;
  }, discordpp::LoggingSeverity::Info);

  m_client->SetStatusChangedCallback([this](discordpp::Client::Status status, discordpp::Client::Error error, int32_t errorDetail) {
    std::cout << "Status changed: " << discordpp::Client::StatusToString(status) << std::endl;
    if (status == discordpp::Client::Status::Ready) {
      discordpp::Activity activity;
      activity.SetType(discordpp::ActivityTypes::Playing);
      activity.SetState("In Competitive Match");
      m_client->UpdateRichPresence(activity, [](discordpp::ClientResult result) {
        if (result.Successful())
          std::cout << "Rich Presence updated successfully!\n";
        else
          std::cerr << "❌ Rich Presence update failed\n";
      });
    }
  });

  auto codeVerifier = m_client->CreateAuthorizationCodeVerifier();
  discordpp::AuthorizationArgs args{};
  args.SetClientId(APPLICATION_ID);
  args.SetScopes(discordpp::Client::GetDefaultPresenceScopes());
  args.SetCodeChallenge(codeVerifier.Challenge());

  m_client->Authorize(args, [this, codeVerifier](auto result, auto code, auto redirectUri) {
    if (!result.Successful()) {
      std::cerr << "❌ Authentication Error: " << result.Error() << std::endl;
      return;
    }

    m_client->GetToken(APPLICATION_ID, code, codeVerifier.Verifier(), redirectUri,
      [this](discordpp::ClientResult result, std::string accessToken, std::string, discordpp::AuthorizationTokenType, int32_t, std::string) {
        if (result.Successful()) {
          m_client->UpdateToken(discordpp::AuthorizationTokenType::Bearer, accessToken,
            [this](discordpp::ClientResult result) {
              if (result.Successful()) {
                m_client->Connect();
              }
            });
        }
      });
  });

  // Start background update loop
  m_thread = std::thread([this]() { this->update(); });
}

void DiscordIntegration::update_status(IntegrationStatus status) {
  if (!m_enabled) return;

  discordpp::Activity activity;

  // Set timestamps
  discordpp::ActivityTimestamps timestamps;
  if (status.m_timestamp != 0) {
    auto now = std::time(nullptr);
    if (status.m_timestamp > now)
      timestamps.SetEnd(status.m_timestamp);
    else
      timestamps.SetStart(status.m_timestamp);
  }
  activity.SetTimestamps(timestamps);

  // Set assets
  discordpp::ActivityAssets assets;
  assets.SetLargeImage("supertux_logo");
  activity.SetAssets(assets);

  m_client->UpdateRichPresence(activity, [](discordpp::ClientResult result) {
    if (!result.Successful()) {
      std::cerr << "❌ Rich Presence update failed\n";
    }
  });
}

void DiscordIntegration::update()
{
  while (m_running)
  {
    discordpp::RunCallbacks();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

void DiscordIntegration::close() {
  if (!m_enabled) return;

  m_running = false;
  if (m_thread.joinable())
    m_thread.join();

  m_client.reset();
  m_enabled = false;
}


#endif
