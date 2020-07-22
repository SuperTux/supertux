
#include <string>

#include "src/discord_rpc.h"

#include <api/external_sdk.hpp>

#ifndef DISCORD_HPP
#define DISCORD_HPP

class DiscordSDK : public ExternalSDK
{
private:
    ~DiscordSDK() {}

public:
	virtual void my_init() override;
	virtual void my_close() override;
	virtual void my_gameLoop() override;
	virtual void my_apiSetStatus(std::string) override;
	virtual void my_apiSetDetails(std::string) override;
	virtual void my_apiSetSmallImage(std::string) override;
	
	std::string getName() override {return "discord";}
	
	// Manage Discord callbacks
	static void handleReady(const DiscordUser*);
};

#endif


