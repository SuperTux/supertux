
// General C++ libs
#include <array>
#include <cassert>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>
#include <string>

// SuperTux libs
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "util/log.hpp"

// SDK/API libs
#include "discord/discord.hpp"

// Local header
#include "external_sdk.hpp"



// List of SDK's
ExternalSDK* sdks[] = {new DiscordSDK};


void ExternalSDK::my_init()
{
	//Log::warn("ExternalSDK", "ERROR : Invoked unimplemented external SDK");
}

// Called once, when the game starts
void ExternalSDK::init()
{
	//Log::info("ExternalSDK", "Initializing all SDK");
	
	for(ExternalSDK* sdk : sdks)
	{
		// FIXME: This toggles ALL SDK'S according to Discord's setting. Should fix before someone passes a week searching for nothing.
		sdk->setEnabled(g_config->enable_discord);
	}
}

// Called once, when the game closes
void ExternalSDK::close()
{
	//Log::info("ExternalSDK", "Closing all SDK");
	
	for(ExternalSDK* sdk : sdks)
	{
		if (sdk->enabled)
			sdk->my_close();
	}
}

// Used for the user config : ExternalSDK::getSDKByName("MySDK")->setEnabled(bool);
ExternalSDK* ExternalSDK::getSDKByName(std::string name)
{
	//Log::info("ExternalSDK", ("Searching for SDK with name '" + name + "'").c_str());
	
	for(ExternalSDK* sdk : sdks)
	{
		if (name.compare(sdk->getName()) == 0)
			return sdk;
	}
	
	return NULL;
}


void ExternalSDK::gameLoop()
{
	// This will spam the output console, only uncomment if necessary
	// Log::info("ExternalSDK", "Call to gameLoop");
	
	for(ExternalSDK* sdk : sdks)
	{
		if (sdk->enabled)
			sdk->my_gameLoop();
	}
}

void ExternalSDK::apiSetStatus(std::string status)
{
	//printf("ExternalSDK : Call to apiSetStatus : '%s'\n", status.c_str());
	
	for(ExternalSDK* sdk : sdks)
	{
		//if (sdk->enabled)
			sdk->my_apiSetStatus(status);
	}
}

void ExternalSDK::apiSetDetails(std::string details)
{
	//printf("ExternalSDK : Call to apiSetDetails : '%s'\n", details.c_str());
	
	for(ExternalSDK* sdk : sdks)
	{
		//if (sdk->enabled)
			sdk->my_apiSetDetails(details);
	}
}

void ExternalSDK::apiSetSmallImage(std::string img)
{
	//printf("ExternalSDK : Call to apiSetSmallImage : '%s'\n", img.c_str());
	
	for(ExternalSDK* sdk : sdks)
	{
		//if (sdk->enabled)
			sdk->my_apiSetSmallImage(img);
	}
}
