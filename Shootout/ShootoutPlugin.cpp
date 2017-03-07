#include "ShootoutPlugin.h"
#include "PortForwardEngine.h"


BAKKESMOD_PLUGIN(ShootoutPlugin, "ShootoutPlugin", "0.2", 0)


GameWrapper* gw;
ConsoleWrapper* cons;

void ShootoutPlugin_consoleNotifier(std::vector<std::string> params) {
	if (params.at(0).compare("openup") == 0) 
	{
		

	}
}

void ShootoutPlugin::onLoad()
{
	gw = gameWrapper;
	cons = console;


	console->registerNotifier("openup", ShootoutPlugin_consoleNotifier);
}


void ShootoutPlugin::onUnload()
{

}