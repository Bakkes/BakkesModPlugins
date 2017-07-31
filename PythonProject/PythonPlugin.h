#pragma once
#pragma comment( lib, "BakkesMod.lib" )
#include "plugin/bakkesmodplugin.h"
#include "plugin/botplugin.h"

class PythonPlugin : public 
#ifdef _AI_BUILD
	bakkesmod::plugin::BotPlugin
#else
	bakkesmod::plugin::BakkesModPlugin
#endif
{
public:
	virtual void onLoad();
	virtual void onUnload();
#ifdef _AI_BUILD
	virtual void on_tick(ControllerInput *input, CarWrapper *localCar, BallWrapper *ball);
#endif
};

