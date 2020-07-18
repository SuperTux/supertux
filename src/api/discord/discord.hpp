
#include <string>

#include "src/discord_rpc.h"

#include <api/external_sdk.hpp>

#ifndef DISCORD_HPP
#define DISCORD_HPP

class DiscordSDK : public ExternalSDK
{

public:
	virtual void my_init();
	virtual void my_close();
	virtual void my_gameLoop();
	virtual void my_apiSetStatus(std::string);
	virtual void my_apiSetDetails(std::string);
	virtual void my_apiSetSmallImage(std::string);
	
	bool getName() {return "DiscordSDK";}
	
	// Manage Discord callbacks
	static void handleReady(const DiscordUser*);
};

#endif


