// Required for the Windows version of the Discord SDK, apparently
#define _CRT_SECURE_NO_WARNINGS

// There are probably some unnecessary includes here, but I'm too bored to remove the extra ones

#include <array>
#include <cassert>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <string.h>

#include <iostream>
#include <fstream>

// STK libs
//#include "utils/log.hpp"

#include "src/discord_rpc.h"

#include "discord.hpp"


// ST-related variables
std::string my_status = "";
std::string my_details = "";
std::string my_sm_img = "";


// ¯\_('-')_/¯ Not sure if it's even needed but the RPC is coded in C
extern "C" {

	static void handleDiscordReady(const DiscordUser* connectedUser)
	{
		//Log::verbose("DiscordSDK", "Ready");
		
		// I recommend not fetching/using/storing Discord identifiers unnecessarily
		// as this could be seen as a privacy invasion for the more concerned users.
		//     ~ Semphris
		
		//printf("\nDiscord: connected to user %s#%s - %s\n",
		//       connectedUser->username,
		//       connectedUser->discriminator,
		//       connectedUser->userId);
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
		//int response = -1;
		//char yn[4];
		printf("\nDiscord: join request from %s#%s - %s\n",
		       request->username,
		       request->discriminator,
		       request->userId);
		 /*
		 do {
		    printf("Accept? (y/n)");
		    if (!prompt(yn, sizeof(yn))) {
		        break;
		    }

		    if (!yn[0]) {
		        continue;
		    }

		    if (yn[0] == 'y') {
		        response = DISCORD_REPLY_YES;
		        break;
		    }

		    if (yn[0] == 'n') {
		        response = DISCORD_REPLY_NO;
		        break;
		    }
		} while (1);
		if (response != -1) {
		    Discord_Respond(request->userId, response);
		}
		*/
		Discord_Respond(request->userId, DISCORD_REPLY_NO);
	}

}






void updateRPC()
{
    DiscordRichPresence discordPresence;
    memset(&discordPresence, 0, sizeof(discordPresence));
    discordPresence.state = my_status.c_str();
    discordPresence.details = my_details.c_str();
    //discordPresence.startTimestamp = 1507665886;
    //discordPresence.endTimestamp = 1507665886000;
    discordPresence.largeImageKey = "supertux_logo";
    discordPresence.largeImageText = "SuperTux";
    discordPresence.smallImageKey = my_sm_img.c_str();
    Discord_UpdatePresence(&discordPresence);
}





// SDK functions

void DiscordSDK::my_init()
{
	// Set Discord event listeners, and start the connection with Discord
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    handlers.ready = handleDiscordReady;
    handlers.disconnected = handleDiscordDisconnected;
    handlers.errored = handleDiscordError;
    handlers.joinGame = handleDiscordJoin;
    handlers.spectateGame = handleDiscordSpectate;
    handlers.joinRequest = handleDiscordJoinRequest;
    Discord_Initialize("733576109744062537", &handlers, 1, NULL);
}

void DiscordSDK::my_close()
{
	// YEET
	Discord_ClearPresence();
}

void DiscordSDK::my_gameLoop()
{
	Discord_RunCallbacks();
	
    // Update the timers if the game is paused
    /*
    long timestamp_this_render = (long)time(nullptr);
    
    if (persistTimer) {
    	
    	long delta = timestamp_this_render - timestamp_last_render;
    	
    	if (timestamp_start != 0)
    		timestamp_start += delta;
    	
    	if (timestamp_end != 0)
    		timestamp_end += delta;
    	
    	updateRPC();
    	
    }
    
    timestamp_last_render = timestamp_this_render;
    */
}

void DiscordSDK::my_apiSetStatus(std::string status)
{
    my_status = status;
    updateRPC();
}

void DiscordSDK::my_apiSetDetails(std::string details)
{
    my_details = details;
    updateRPC();
}

void DiscordSDK::my_apiSetSmallImage(std::string img)
{
    my_sm_img = img;
    updateRPC();
}

/*
void DiscordSDK::my_apiSetPause(bool new_persist)
{
	persistTimer = new_persist;
}
*/
