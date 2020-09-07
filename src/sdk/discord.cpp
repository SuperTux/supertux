/*
    This is a simple example in C of using the rich presence API asynchronously.
*/

#define _CRT_SECURE_NO_WARNINGS /* thanks Microsoft */

extern "C" {

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "discord_rpc.h"
#include "sdk/discord.hpp"

static const char* APPLICATION_ID = "733576109744062537";
static int FrustrationLevel = 0;
static int64_t StartTime;
static int SendPresence = 1;

static void updateDiscordPresence()
{
  char buffer[256];
  DiscordRichPresence discordPresence;
  memset(&discordPresence, 0, sizeof(discordPresence));
  discordPresence.state = "West of House";
  sprintf(buffer, "Frustration level: %d", FrustrationLevel);
  discordPresence.details = buffer;
  discordPresence.startTimestamp = StartTime;
  discordPresence.endTimestamp = time(0) + 5 * 60;
  discordPresence.largeImageKey = "canary-large";
  discordPresence.smallImageKey = "ptb-small";
  discordPresence.partyId = "party1234";
  discordPresence.partySize = 1;
  discordPresence.partyMax = 6;
  discordPresence.matchSecret = "xyzzy";
  discordPresence.joinSecret = "join";
  discordPresence.spectateSecret = "look";
  discordPresence.instance = 0;
  Discord_UpdatePresence(&discordPresence);
}

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
    char yn[4];
    
    printf("\nDiscord: join request from %s#%s - %s\n",
           request->username,
           request->discriminator,
           request->userId);
    
    response = false ? DISCORD_REPLY_YES : DISCORD_REPLY_NO;
    
    if (response != -1) {
        Discord_Respond(request->userId, response);
    }
}

static void discordInit()
{
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    handlers.ready = handleDiscordReady;
    handlers.disconnected = handleDiscordDisconnected;
    handlers.errored = handleDiscordError;
    handlers.joinGame = handleDiscordJoin;
    handlers.spectateGame = handleDiscordSpectate;
    handlers.joinRequest = handleDiscordJoinRequest;
    Discord_Initialize(APPLICATION_ID, &handlers, 1, NULL);
}

void discord_launch()
{
    discordInit();
    updateDiscordPresence();
}

void discord_update()
{
#ifdef DISCORD_DISABLE_IO_THREAD
    Discord_UpdateConnection();
#endif
    Discord_RunCallbacks();
}

void discord_close()
{
    Discord_ClearPresence();
    Discord_Shutdown();
}

}

/* EOF */

