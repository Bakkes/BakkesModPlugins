#include "DefenderPluginR.h"

#include "helpers.h"

BAKKESMOD_PLUGIN(DefenderPluginR, "Defender plugin", "0.1", PLUGINTYPE_FREEPLAY)

GameWrapper* gw;
ConsoleWrapper* cons;

bool defenderActive = false;


void defenderplugin_ConsoleNotifier(std::vector<std::string> params) {
	string command = params.at(0);

	if (!gw->IsInTutorial())
		return;

	if (command.compare("defender_start") == 0)
	{
		defenderActive = true;
		cons->executeCommand("training_scoring 0");
		go();
	}
	else if (command.compare("defender_stop") == 0)
	{
		defenderActive = false;
	}

}

void DefenderPluginR::onLoad()
{
	gw = gameWrapper;
	cons = console;
	srand(time(NULL));
	console->registerNotifier("defender_start", defenderplugin_ConsoleNotifier);
	console->registerNotifier("defender_stop", defenderplugin_ConsoleNotifier);
	console->registerCvar("defender_shotspeed", "(800, 1100)");
	console->registerCvar("defender_cooldown", "(3000, 6000)"); //Cooldown in MS
}

void DefenderPluginR::onUnload()
{
	console->unregisterNotifier("defender_start", defenderplugin_ConsoleNotifier);
	console->unregisterNotifier("defender_stop", defenderplugin_ConsoleNotifier);
}