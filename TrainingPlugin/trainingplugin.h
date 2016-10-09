#pragma once
#pragma comment( lib, "BakkesMod.lib" )
#include "plugin/bakkesmodplugin.h"
#include "helpers.h"

GameWrapper* gw;
ConsoleWrapper* cons;
#include "jsonshot.h"
class TrainingPlugin : public bakkesmod::plugin::BakkesModPlugin
{
public:
	virtual void onLoad();
	virtual void onUnload();
};
