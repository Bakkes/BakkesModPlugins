#pragma once
#pragma comment( lib, "..\\ReleaseLib\\BakkesMod.lib" )
#include "plugin/bakkesmodplugin.h"

class TouchPlugin : public bakkesmod::plugin::BakkesModPlugin
{
public:
	virtual void onLoad();
	virtual void onUnload();
};

