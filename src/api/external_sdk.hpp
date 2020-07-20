
#include <string>

#ifndef EXTERNAL_SDK_HPP
#define EXTERNAL_SDK_HPP

class ExternalSDK
{
public:

	// Called by the game for the SDK
	static void init();
	static void close();
	static void gameLoop();
	static void apiSetStatus(std::string);
	static void apiSetDetails(std::string);
	static void apiSetSmallImage(std::string);
	
	static ExternalSDK* getSDKByName(std::string name);
	
	virtual void my_init() = 0;
	virtual void my_close() = 0;
	virtual void my_gameLoop() = 0;
	virtual void my_apiSetStatus(std::string) = 0;
	virtual void my_apiSetDetails(std::string) = 0;
	virtual void my_apiSetSmallImage(std::string) = 0;
	
	// Controls whether or not the SDK is activated. If not,
	// then no code from the corresponding SDK must be ran.
	// Must be false by default and manually enabled per the
	// configuration (e. g. it must not be run unless the
	// player manually says otherwise).
	bool enabled = false;
	bool isEnabled() {return this->enabled;}
	void setEnabled(bool new_enabled)
	{
		if (this->enabled != new_enabled) {
			if (new_enabled) {
				this->my_init();
			} else {
				this->my_close();
			}
			this->enabled = new_enabled;
		}
	}
	
	// Returns the name of the SDK. Used as identifier.
	virtual std::string getName() = 0;
};

#endif


